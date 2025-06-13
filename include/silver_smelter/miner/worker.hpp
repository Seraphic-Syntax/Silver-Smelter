#pragma once

#include "silver_smelter/net/stratum.hpp" // This now correctly includes StratumV2Job
#include <vector>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>

class Miner {
public:
    // The constructor takes ownership of a StratumClient.
    // The default argument for num_threads is only specified here in the header.
    Miner(std::unique_ptr<StratumClient> client, int num_threads = 0);
    ~Miner();

    void start();
    void stop();

    // The callback method for the StratumClient to call.
    // It now uses the StratumV2Job struct.
    void on_new_job(StratumV2Job job);

private:
    void run_worker(int thread_id);

    // --- Member Variables ---
    std::unique_ptr<StratumClient> m_client;
    
    int m_num_threads;
    std::vector<std::thread> m_threads;
    
    // This atomic flag tells workers to stop and get new work.
    std::atomic<bool> m_new_job_available;
    std::atomic<bool> m_is_running;

    // The current job is shared between threads, so it needs a mutex.
    // It is now a pointer to a StratumV2Job.
    std::shared_ptr<StratumV2Job> m_current_job;
    std::mutex m_job_mutex;
};