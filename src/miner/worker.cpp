#include "silver_smelter/miner/worker.hpp"
#include "silver_smelter/util/log.hpp"
#include <iostream>

Miner::Miner(std::unique_ptr<StratumClient> client, int num_threads = 0)
    : m_client(std::move(client)), m_is_running(false)
{
    if (num_threads <= 0) {
        m_num_threads = std::thread::hardware_concurrency();
    } else {
        m_num_threads = num_threads;
    }
    Log::info("Miner configured to use " + std::to_string(m_num_threads) + " threads.");
}

Miner::~Miner() {
    if (m_is_running) {
        stop();
    }
}

void Miner::start() {
    m_is_running = true;
    m_client->connect();

    for (int i = 0; i < m_num_threads; ++i) {
        m_threads.emplace_back(&Miner::run_worker, this, i);
    }
    Log::info("All miner threads started.");
}

void Miner::stop() {
    m_is_running = false;
    Log::warn("Stopping miner threads...");
    for (auto& thread : m_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    Log::info("All miner threads stopped.");
}

void Miner::run_worker(int thread_id) {
    Log::info("Worker thread " + std::to_string(thread_id) + " starting.");
    
    // Each worker gets the current job
    auto job_