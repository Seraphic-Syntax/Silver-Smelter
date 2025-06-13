#pragma once

#include "silver_smelter/net/stratum.hpp"
#include <vector>
#include <thread>
#include <atomic>
#include <memory>

class Miner {
public:
    Miner(std::unique_ptr<StratumClient> client, int num_threads);
    ~Miner();

    void start();
    void stop();

private:
    void run_worker(int thread_id);

    std::unique_ptr<StratumClient> m_client;
    int m_num_threads;
    std::vector<std::thread> m_threads;
    std::atomic<bool> m_is_running;
};