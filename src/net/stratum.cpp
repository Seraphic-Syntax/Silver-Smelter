// Replace content of: src/net/stratum.cpp
#include "silver_smelter/net/stratum.hpp"
#include "silver_smelter/util/log.hpp"
#include <iostream>

namespace asio = boost::asio;
using asio::ip::tcp;

StratumClient::StratumClient(asio::io_context& ioc, const std::string& host, const std::string& port, const std::string& user)
    : m_ioc(ioc),
      m_socket(ioc),
      m_resolver(ioc),
      m_host(host),
      m_port(port),
      m_user(user),
      m_session_id(0),
      m_header_buffer(sizeof(MessageHeader)) // Allocate buffer for one header
{}

void StratumClient::on_new_job(JobCallback callback) {
    m_job_callback = std::move(callback);
}

void StratumClient::connect() {
    Log::info("Resolving " + m_host + ":" + m_port + "...");
    m_resolver.async_resolve(m_host, m_port, 
        [this](const boost::system::error_code& ec, tcp::resolver::results_type endpoints) {
        if (ec) {
            Log::error("Resolve failed: " + ec.message());
            return;
        }
        asio::async_connect(m_socket, endpoints, 
            [this](const boost::system::error_code& ec, const tcp::endpoint& endpoint) {
            if (ec) {
                Log::error("Connect failed to " + endpoint.address().to_string() + ": " + ec.message());
                return;
            }
            Log::success("Connection established to " + endpoint.address().to_string() + "!");
            send_subscribe();
            do_read_header(); // Start the read loop
        });
    });
}

void StratumClient::send_subscribe() {
    Subscribe sub_msg{};
    // V2 requires a 32-byte public key. For unencrypted, this is all zeros.
    memset(sub_msg.pool_public_key, 0, sizeof(sub_msg.pool_public_key));
    
    // Copy user agent and identity, ensuring null padding.
    strncpy(sub_msg.user_agent, "Silver-Smelter/0.2.0", sizeof(sub_msg.user_agent) - 1);
    strncpy(sub_msg.user_identity, m_user.c_str(), sizeof(sub_msg.user_identity) - 1);
    
    sub_msg.max_extranonce_size = 4; // We can handle a 4-byte extranonce

    MessageHeader header{0x02, 0, sizeof(Subscribe)};
    
    std::vector<char> full_message(sizeof(MessageHeader) + sizeof(Subscribe));
    memcpy(full_message.data(), &header, sizeof(MessageHeader));
    memcpy(full_message.data() + sizeof(MessageHeader), &sub_msg, sizeof(Subscribe));

    Log::info("Sending Subscribe message...");
    do_write(full_message.data(), full_message.size());
}

void StratumClient::do_read_header() {
    asio::async_read(m_socket, asio::buffer(m_header_buffer),
        [this](const boost::system::error_code& ec, std::size_t bytes) {
            on_read_header(ec, bytes);
        });
}

void StratumClient::on_read_header(const boost::system::error_code& ec, std::size_t /*bytes*/) {
    if (ec) {
        Log::error("Read header failed: " + ec.message());
        stop();
        return;
    }
    const MessageHeader* header = reinterpret_cast<const MessageHeader*>(m_header_buffer.data());
    if (header->msg_len > 0) {
        do_read_body(header->msg_len);
    } else {
        // Message has no body, process it directly
        dispatch_message(*header, {});
        do_read_header(); // Wait for next message
    }
}

void StratumClient::do_read_body(uint16_t body_length) {
    m_body_buffer.resize(body_length);
    asio::async_read(m_socket, asio::buffer(m_body_buffer),
        [this](const boost::system::error_code& ec, std::size_t bytes) {
            on_read_body(ec, bytes);
        });
}

void StratumClient::on_read_body(const boost::system::error_code& ec, std::size_t /*bytes*/) {
    if (ec) {
        Log::error("Read body failed: " + ec.message());
        stop();
        return;
    }
    const MessageHeader* header = reinterpret_cast<const MessageHeader*>(m_header_buffer.data());
    dispatch_message(*header, m_body_buffer);
    do_read_header(); // Loop back to wait for the next header
}

void StratumClient::dispatch_message(const MessageHeader& header, const std::vector<char>& body) {
    Log::info("Dispatching message of type: " + std::to_string(header.msg_type));
    switch (header.msg_type) {
        case SETUP_CONNECTION_SUCCESS:
            handle_setup_connection_success(body);
            break;
        case NEW_MINING_JOB:
            handle_new_mining_job(body);
            break;
        default:
            Log::warn("Received unhandled message type: " + std::to_string(header.msg_type));
            break;
    }
}

void StratumClient::handle_setup_connection_success(const std::vector<char>& body) {
    const SetupConnectionSuccess* msg = reinterpret_cast<const SetupConnectionSuccess*>(body.data());
    m_session_id = msg->session_id;
    Log::success("Stratum V2 connection successful! Session ID: " + std::to_string(m_session_id));
}

void StratumClient::handle_new_mining_job(const std::vector<char>& body) {
    const NewMiningJob* msg = reinterpret_cast<const NewMiningJob*>(body.data());
    
    StratumV2Job job;
    job.job_id = msg->job_id;
    
    // Construct the block header from the NewMiningJob message
    job.header.version = msg->version;
    job.header.bits = msg->bits;
    job.header.nonce = 0; // We will iterate this
    memcpy(job.header.prev_block_hash.data(), msg->prev_block_hash, 32);

    // The MOST COMPLEX part of a real miner is building the true merkle root
    // from coinbase prefix/suffix and the merkle branch.
    // FOR NOW, we will use a placeholder merkle root.
    // TODO: Implement real coinbase and merkle root construction.
    memset(job.header.merkle_root.data(), 0, 32); // Placeholder
    job.header.timestamp = time(0); // Placeholder

    job.target = calculate_target_from_bits(job.header.bits);

    Log::success("Received new V2 mining job ID: " + std::to_string(job.job_id));
    if (m_job_callback) {
        m_job_callback(job);
    }
}

void StratumClient::do_write(const void* data, size_t size) {
    asio::async_write(m_socket, asio::buffer(data, size),
        [](const boost::system::error_code&, std::size_t){});
}

void StratumClient::submit_share(uint32_t job_id, uint32_t nonce) {
    SubmitShares share_msg{};
    share_msg.session_id = m_session_id;
    share_msg.job_id = job_id;
    share_msg.nonce = nonce;

    MessageHeader header{0x02, 6, sizeof(SubmitShares)};

    std::vector<char> full_message(sizeof(MessageHeader) + sizeof(SubmitShares));
    memcpy(full_message.data(), &header, sizeof(MessageHeader));
    memcpy(full_message.data() + sizeof(MessageHeader), &share_msg, sizeof(SubmitShares));
    
    Log::success("Submitting share for job " + std::to_string(job_id) + " with nonce " + std::to_string(nonce));
    do_write(full_message.data(), full_message.size());
}

void StratumClient::stop() {
    boost::system::error_code ec;
    if (m_socket.is_open()) {
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        m_socket.close(ec);
    }
}