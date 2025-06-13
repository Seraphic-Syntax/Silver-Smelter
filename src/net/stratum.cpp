#include "silver_smelter/net/stratum.hpp"
#include "silver_smelter/util/log.hpp"
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp> // For boost::bind used in older examples, though lambdas are preferred now
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
      m_read_buffer()
{}

void StratumClient::on_new_job(JobCallback callback) {
    m_job_callback = std::move(callback);
}

void StratumClient::connect() {
    Log::info("Resolving " + m_host + ":" + m_port + "...");
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
    asio::async_connect(m_socket, endpoints,
        [this](const boost::system::error_code& ec, const tcp::endpoint& endpoint) {
            on_connect(ec, endpoint);
        });
}

// THIS IS THE CORRECTED FUNCTION SIGNATURE
void StratumClient::on_connect(const boost::system::error_code& ec, const tcp::resolver::results_type::endpoint_type& endpoint) {
    if (ec) {
        Log::error("Connect failed to " + endpoint.address().to_string() + ": " + ec.message());
        return;
    }
    Log::success("Connection established to " + endpoint.address().to_string() + "!");
    
    do_read(); // Start listening for data

    // This is a placeholder for Stratum V1. A real implementation needs JSON parsing.
    std::string subscribe_msg = "{\"id\": 1, \"method\": \"mining.subscribe\", \"params\": [\"Silver-Smelter/0.1.0\"]}\\n";
    do_write(subscribe_msg);
    std::string auth_msg = "{\"id\": 2, \"method\": \"mining.authorize\", \"params\": [\"" + m_user + "\", \"x\"]}\\n";
    do_write(auth_msg);
}

void StratumClient::do_read() {
    // Read until a newline character is encountered.
    asio::async_read_until(m_socket, m_read_buffer, '\n',
        [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            on_read(ec, bytes_transferred);
        });
}

void StratumClient::on_read(const boost::system::error_code& ec, std::size_t bytes_transferred) {
    if (!ec) {
        std::istream is(&m_read_buffer);
        std::string message;
        std::getline(is, message); // Extract the line from the buffer
        
        process_message(message);
        
        do_read(); // Go back to waiting for the next message
    } else if (ec != asio::error::eof) {
        Log::error("Read error: " + ec.message());
        stop();
    }
}

void StratumClient::process_message(const std::string& message) {
    Log::info("From Pool: " + message);

    // TODO: This is where you would parse a REAL Stratum message using a JSON library.
    // We will continue to simulate receiving a new job for now.
    if (m_job_callback) {
        Log::info("Simulating new job from pool message...");
        StratumJob job;
        job.job_id = "job" + std::to_string(rand() % 1000);
        job.header.version = 0x20000000;
        std::fill(job.header.prev_block_hash.begin(), job.header.prev_block_hash.end(), 0xaa);
        std::fill(job.header.merkle_root.begin(), job.header.merkle_root.end(), 0xbb);
        job.header.timestamp = time(0);
        job.header.bits = 0x1d00ffff; // Easy difficulty for testing
        job.header.nonce = 0;
        job.target = calculate_target_from_bits(job.header.bits);
        
        m_job_callback(job);
    }
}

void StratumClient::do_write(const std::string& message) {
    asio::async_write(m_socket, asio::buffer(message),
        [](const boost::system::error_code& /*ec*/, std::size_t /*bytes_transferred*/) {
            // Can add logging here if write fails
        });
}

void StratumClient::submit_share(const std::string& job_id, uint32_t nonce, const std::string& extranonce2, const std::string& ntime) {
    Log::success("Submitting share for job " + job_id + " with nonce " + std::to_string(nonce));
    
    // This is still a placeholder JSON message for Stratum V1.
    std::string submit_msg = "{\"id\": 4, \"method\": \"mining.submit\", \"params\": [\"" + m_user + "\", \"" + job_id + "\", \"" + extranonce2 + "\", \"" + ntime + "\", \"" + std::to_string(nonce) + "\"]}\\n";
    do_write(submit_msg);
}

void StratumClient::stop() {
    boost::system::error_code ec;
    if (m_socket.is_open()) {
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        m_socket.close(ec);
    }
}