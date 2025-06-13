#pragma once

#include "v2_protocol.hpp" // Our header for V2 structs
#include "silver_smelter/core/block.hpp"
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <boost/asio.hpp>

// This is the new job structure that aligns with the NewMiningJob message
struct StratumV2Job {
    uint32_t job_id;
    BlockHeader header; // We will construct this from the NewMiningJob fields
    target_t target;
};

class StratumClient {
public:
    using JobCallback = std::function<void(StratumV2Job)>;

    // The constructor is updated to accept the pool's public key string.
    StratumClient(boost::asio::io_context& ioc, const std::string& host, const std::string& port, const std::string& user, const std::string& pool_pub_key);

    void on_new_job(JobCallback callback);
    void connect();
    void submit_share(uint32_t job_id, uint32_t nonce);
    void stop();

private:
    // Main read loop logic for binary protocols
    void do_read_header();
    void on_read_header(const boost::system::error_code& ec, std::size_t bytes);
    void do_read_body(uint16_t body_length);
    void on_read_body(const boost::system::error_code& ec, std::size_t bytes);
    
    // Sends a raw message to the pool
    void do_write(const void* data, size_t size);
    
    // Message handling
    void dispatch_message(const MessageHeader& header, const std::vector<char>& body);
    void handle_setup_connection_success(const std::vector<char>& body);
    void handle_new_mining_job(const std::vector<char>& body);

    // V2 specific actions
    void send_subscribe();

    // --- Member Variables ---
    boost::asio::io_context& m_ioc;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::resolver m_resolver;
    
    std::string m_host;
    std::string m_port;
    std::string m_user;
    std::string m_pool_pub_key; // Added member to store the pool's public key

    uint32_t m_session_id; // V2 uses a session ID
    JobCallback m_job_callback;

    // Buffers for reading network data
    std::vector<char> m_header_buffer;
    std::vector<char> m_body_buffer;
};