#pragma once

#include "silver_smelter/net/stratum.hpp"
#include <vector>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>

class Miner {
public:
    Miner(int num_threads);
    ~Miner();

    void start();
    void stop();

    // The callback method for the StratumClient to call.
    void on_new_job(StratumJob job);

private:
    void run_worker(int thread_id);

    int m_num_threads;
    std::vector<std::thread> m_threads;
    
    // This atomic flag tells workers to stop and get new work.
    std::atomic<bool> m_new_job_available;
    std::atomic<bool> m_is_running;

    // The current job is shared between threads, so it needs a mutex.
    std::shared_ptr<StratumJob> m_current_job;
    std::mutex m_job_mutex;
};
