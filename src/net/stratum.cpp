#include "silver_smelter/net/stratum.hpp"
#include "silver_smelter/util/log.hpp"
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp> // For boost::bind used in older examples, though lambdas are preferred now

namespace asio = boost::asio;
using asio::ip::tcp;

StratumClient::StratumClient(asio::io_context& ioc, const std::string& host, const std::string& port, const std::string& user)
    : m_ioc(ioc),
      m_socket(ioc),
      m_resolver(ioc),
      m_host(host),
      m_port(port),
      m_user(user),
      m_read_buffer(4096) // Allocate a 4KB read buffer
{// In file: src/net/stratum.cpp
    Log::info("StratumClient created for " + m_host + ":" + m_port + " as user " + m_user);
}

void StratumClient::on_resolve(const boost::system::error_code& ec, tcp::resolver::results_type endpoints) {
    if (ec) {
        Log::error("Resolve failed: " + ec.message());
        return;
    }
    Log::info("Connecting to endpoint...");
    // Step 2: Attempt to connect to the first resolved endpoint.
    asio::async_connect(m_socket, endpoints,
        [this](const boost::system::error_code& ec, const tcp::endpoint& /*endpoint*/) { // <-- also add /*endpoint*/ to fix unused parameter warning
            on_connect(ec);
        });
}

void StratumClient::on_connect(const boost::system::error_code& ec) {
    if (ec) {
        Log::error("Connect failed: " + ec.message());
        // TODO: Implement reconnect logic here.
        return;
    }
    Log::success("Connection established!");

void StratumClient::on_new_job(JobCallback callback) {
    m_job_callback = std::move(callback);
}

void StratumClient::connect() {
    Log::info("Resolving " + m_host + ":" + m_port + "...");
    // Step 1: Resolve the hostname to a list of IP addresses.
    m_resolver.async_resolve(m_host, m_port,
        [this](const boost::system::error_code& ec, tcp::resolver::results_type endpoints) {
            on_resolve(ec, endpoints);
        });
}

void StratumClient::on_resolve(const boost::system::error_code& ec, tcp::resolver::results_type endpoints) {
    if (ec) {
        Log::error("Resolve failed: " + ec.message());
        return;
    }
    Log::info("Connecting to endpoint...");
    // Step 2: Attempt to connect to the first resolved endpoint.
    asio::async_connect(m_socket, endpoints,
        [this](const boost::system::error_code& ec, const tcp::endpoint& endpoint) {
            on_connect(ec);
        });
}

void StratumClient::on_connect(const boost::system::error_code& ec) {
    if (ec) {
        Log::error("Connect failed: " + ec.message());
        // TODO: Implement reconnect logic here.
        return;
    }
    Log::success("Connection established!");
    // Step 3: We are connected. Start the read loop to listen for messages.
    do_read();

    // Step 4: Subscribe to the pool.
    // TODO: This should be a real Stratum V2 binary message.
    std::string subscribe_msg = "{\"id\": 1, \"method\": \"mining.subscribe\", \"params\": [\"Silver-Smelter/0.1.0\"]}\\n";
    do_write(subscribe_msg);
    std::string auth_msg = "{\"id\": 2, \"method\": \"mining.authorize\", \"params\": [\"" + m_user + "\", \"x\"]}\\n";
    do_write(auth_msg);
}

void StratumClient::do_read() {
    // Step 5: Wait for data from the server.
    m_socket.async_read_some(asio::buffer(m_read_buffer),
        [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            on_read(ec, bytes_transferred);
        });
}

void StratumClient::on_read(const boost::system::error_code& ec, std::size_t bytes_transferred) {
    if (!ec) {
        std::string message(m_read_buffer.begin(), m_read_buffer.begin() + bytes_transferred);
        // This is a simplification. A real client would need a proper buffer that
        // handles multiple messages arriving in one read, or one message split across multiple reads.
        process_message(message);

        // Step 6: Go back to waiting for more data.
        do_read();
    } else if (ec != asio::error::eof) {
        Log::error("Read error: " + ec.message());
        stop();
    }
}

void StratumClient::process_message(const std::string& message) {
    // This is where you would parse the JSON or binary message from the pool.
    // For now, we simulate receiving a new job.
    Log::info("Received from pool: " + message);

    // TODO: This is where you would parse a REAL Stratum message.
    // For now, we will just trigger a new mock job every time we get any message.
    if (m_job_callback) {
        Log::info("Simulating new job from pool message...");
        StratumJob job;
        job.job_id = "job" + std::to_string(rand() % 1000); // Random job ID
        job.header.version = 0x20000000;
        std::fill(job.header.prev_block_hash.begin(), job.header.prev_block_hash.end(), 0xaa);
        std::fill(job.header.merkle_root.begin(), job.header.merkle_root.end(), 0xbb);
        job.header.timestamp = time(0);
        job.header.bits = 0x1d00ffff; // Easy difficulty for testing
        job.header.nonce = 0;
        job.target = calculate_target_from_bits(job.header.bits);
        
        m_job_callback(job); // Send the new job to the Miner class.
    }
}


void StratumClient::do_write(const std::string& message) {
    asio::async_write(m_socket, asio::buffer(message),
        [](const boost::system::error_code& /*ec*/, std::size_t /*bytes_transferred*/) {
            // Can add logging here if write fails
        });
}

void StratumClient::submit_share(const std::string& job_id, uint32_t nonce) {
    Log::success("Submitting share for job " + job_id + " with nonce " + std::to_string(nonce));
    // TODO: This should be a real Stratum V2 binary message.
    std::string submit_msg = "{\"id\": 4, \"method\": \"mining.submit\", \"params\": [\"" + m_user + "\", \"" + job_id + "\", \"00000000\", \"00000000\", \"" + std::to_string(nonce) + "\"]}\\n";
    do_write(submit_msg);
}

void StratumClient::stop() {
    if (m_socket.is_open()) {
        m_socket.close();
    }
}
void StratumClient::stop() {
    if (m_socket.is_open()) {
        m_socket.close();
        Log::info("Stratum client stopped.");
    } else {
        Log::warn("Stratum client was already stopped.");
    }
// In file: src/net/stratum.cpp

// ... all the other functions ...

void StratumClient::submit_share(const std::string& job_id, uint32_t nonce) {
    Log::success("Submitting share for job " + job_id + " with nonce " + std::to_string(nonce));
    // TODO: This should be a real Stratum V2 binary message.
    std::string submit_msg = "{\"id\": 4, \"method\": \"mining.submit\", \"params\": [\"" + m_user + "\", \"" + job_id + "\", \"00000000\", \"00000000\", \"" + std::to_string(nonce) + "\"]}\\n";
    do_write(submit_msg);
}

void StratumClient::stop() {
    boost::system::error_code ec; // To ignore errors on close
    if (m_socket.is_open()) {
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        m_socket.close(ec);
    }
}
    Log::info("Stratum client stopped.");
}