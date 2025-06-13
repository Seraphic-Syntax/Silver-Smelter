#include "silver_smelter/miner/worker.hpp"
#include "silver_smelter/util/log.hpp"
#include <boost/asio.hpp>
#include <thread>
#include <iostream>

int main(int argc, char* argv[]) {
    Log::info("Silver-Smelter Bitcoin Miner starting...");

    // --- Configuration from your Stratum V2 URI ---
    const std::string host = "v2.us-east.stratum.braiins.com";      // <-- From the URI
    const std::string port = "3334";                                // Standard port for stratum2+tcp
    const std::string user = "Seraphic-Syntax.Silver-Smelter";             // Your Braiins Pool username
    const std::string pool_public_key_str = "u95GEReVMjK6k5YqiSFNqqTnKU4ypU2Wm8awa6tmbmDmk1bWt"; // <-- From the URI
    // ---------------------------------------------------

    Log::info("Pool: " + host + ":" + port);
    Log::info("User: " + user);

    // --- Setup Asynchronous I/O ---
    boost::asio::io_context ioc;

    // --- Create Miner Components ---
    // Create a StratumClient, now passing all 5 arguments including the public key.
    auto client = std::make_unique<StratumClient>(ioc, host, port, user, pool_public_key_str);

    // Create the Miner, giving it ownership of the client.
    Miner miner(std::move(client));

    // --- Start Threads ---
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