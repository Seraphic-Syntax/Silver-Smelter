// Replace the contents of this file: include/silver_smelter/miner/worker.hpp

#pragma once

#include "silver_smelter/net/stratum.hpp"
#include <vector>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>

class Miner {
public:
    // This constructor must match the one in worker.cpp
    Miner(std::unique_ptr<StratumClient> client, int num_threads = 0);
    ~Miner();

    void start();
    void stop();

    // The callback method for the StratumClient to call.
    void on_new_job(StratumJob job);

private:
    void run_worker(int thread_id);

    // This member was missing from the old header file
    std::unique_ptr<StratumClient> m_client;
    
    int m_num_threads;
    std::vector<std::thread> m_threads;
    
    std::atomic<bool> m_new_job_available;
    std::atomic<bool> m_is_running;

    std::shared_ptr<StratumJob> m_current_job;
    std::mutex m_job_mutex;
};