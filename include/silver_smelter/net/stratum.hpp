#pragma once

#include "silver_smelter/core/block.hpp"
#include <string>
#include <functional> // For std::function (our callback)

// Forward declarations for Boost.Asio types to keep header clean
namespace boost::asio {
    class io_context;
    namespace ip {
        class tcp;
    }
}

// Data received from the pool for a single mining job
struct StratumJob {
    std::string job_id;
    BlockHeader header;
    target_t target;
};

// A real, asynchronous Stratum client.
class StratumClient {
public:
    // The callback type: a function that takes a StratumJob and returns void.
    using JobCallback = std::function<void(StratumJob)>;

    StratumClient(boost::asio::io_context& ioc, const std::string& host, const std::string& port, const std::string& user);

    // Set the callback function that will be called when a new job arrives.
    void on_new_job(JobCallback callback);

    // Asynchronously connect to the server.
    void connect();

    // Asynchronously submit a found share.
    void submit_share(const std::string& job_id, uint32_t nonce);

    // Stop the client and close the connection.
    void stop();

private:
    // These methods handle the asynchronous connection steps.
    void on_resolve(const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::results_type endpoints);
    void on_connect(const boost::system::error_code& ec);

    // These methods handle reading data from the server.
    void do_read();
    void on_read(const boost::system::error_code& ec, std::size_t bytes_transferred);
    
    // This method handles writing data to the server.
    void do_write(const std::string& message);

    // This method will parse messages from the pool.
    void process_message(const std::string& message);

    boost::asio::io_context& m_ioc;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::resolver m_resolver;
    std::string m_host;
    std::string m_port;
    std::string m_user;
    std::vector<char> m_read_buffer;

    JobCallback m_job_callback;
};
