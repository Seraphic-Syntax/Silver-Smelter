#include "silver_smelter/miner/worker.hpp"
#include "silver_smelter/util/log.hpp"
#include <boost/asio.hpp>
#include <thread>
#include <iostream>

int main(int argc, char* argv[]) {
    Log::info("Silver-Smelter Bitcoin Miner starting...");

    // --- Configuration ---
    // In a real app, these would come from command-line arguments (cxxopts).
    const std::string host = "stratum.braiins.com"; // Using old Stratum V1 for this example
    const std::string port = "3333";
    const std::string user = "Seraphic-Syntax.worker1";
    Log::info("Pool: " + host + ":" + port);
    Log::info("User: " + user);

    // --- Setup Asynchronous I/O ---
    // The io_context is the heart of Boost.Asio. All async operations run on it.
    boost::asio::io_context ioc;

    // --- Create Miner Components ---
    // Create a StratumClient, passing it the io_context.
    auto client = std::make_unique<StratumClient>(ioc, host, port, user);

    // Create the Miner, giving it ownership of the client.
    Miner miner(std::move(client));

    // --- Start Threads ---
    // The network operations need their own thread to run on.
    // The ioc.run() call will block this thread and process async events.
    std::thread network_thread([&ioc]() {
        try {
            ioc.run();
        } catch (const std::exception& e) {
            Log::error("Network thread exception: " + std::string(e.what()));
        }
    });
    Log::info("Network thread started.");
    
    // Start the miner. This will connect and launch the worker threads.
    miner.start();

    // --- Wait for shutdown signal ---
    Log::info("Miner is running. Press Enter to stop.");
    std::cin.get(); // Wait for user to press Enter.

    // --- Graceful Shutdown ---
    Log::warn("Shutdown initiated by user.");
    
    // Stop the miner (signals workers, closes socket).
    miner.stop();

    // Stop the io_context. This will unblock ioc.run() in the network thread.
    ioc.stop();

    // Wait for the network thread to finish its cleanup.
    if(network_thread.joinable()) {
        network_thread.join();
    }

    Log::success("Silver-Smelter has shut down cleanly.");
    return 0;
}
