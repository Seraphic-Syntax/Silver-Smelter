#include "silver_smelter/miner/worker.hpp"
#include "silver_smelter/util/log.hpp"
#include <iostream>
#include <ctime>

// The Miner constructor takes ownership of the StratumClient.
// The default argument for num_threads is only in the .hpp file, not here.
Miner::Miner(std::unique_ptr<StratumClient> client, int num_threads)
    : m_client(std::move(client)),
      m_is_running(false),
      m_new_job_available(false)
{
    if (num_threads <= 0) {
        // Use the number of concurrent threads supported by the hardware.
        m_num_threads = std::thread::hardware_concurrency();
    } else {
        m_num_threads = num_threads;
    }
    Log::info("Miner configured to use " + std::to_string(m_num_threads) + " worker threads.");
}

Miner::~Miner() {
    if (m_is_running) {
        stop();
    }
}

void Miner::start() {
    m_is_running = true;

    // Set up the callback. The miner's on_new_job method will be called
    // by the client whenever a new job arrives from the network.
    // We use a lambda to correctly bind the 'this' pointer and the new job type.
    m_client->on_new_job([this](StratumV2Job job) {
        this->on_new_job(job);
    });

    // Start the client connection process.
    m_client->connect();

    // Launch the worker threads.
    for (int i = 0; i < m_num_threads; ++i) {
        m_threads.emplace_back(&Miner::run_worker, this, i);
    }
    Log::info("All miner threads started.");
}

void Miner::stop() {
    m_is_running = false; // Signal all threads to stop their main loop.
    m_client->stop();     // Close the network connection.
    Log::warn("Stopping miner threads...");
    for (auto& thread : m_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    Log::info("All miner threads have been stopped.");
}

// The callback now accepts the StratumV2Job struct.
void Miner::on_new_job(StratumV2Job job) {
    Log::success("New V2 job received by Miner: " + std::to_string(job.job_id));
    // Lock the mutex to safely update the shared job pointer.
    std::lock_guard<std::mutex> lock(m_job_mutex);
    m_current_job = std::make_shared<StratumV2Job>(job);
    
    // Set the flag to notify all worker threads that new work is ready.
    m_new_job_available = true;
}

void Miner::run_worker(int thread_id) {
    Log::info("Worker thread " + std::to_string(thread_id) + " starting.");
    
    while (m_is_running) {
        std::shared_ptr<StratumV2Job> local_job;

        // Wait for a job to be available.
        {
            std::unique_lock<std::mutex> lock(m_job_mutex);
            if (!m_current_job) {
                // If there's no job at all, wait a moment and check again.
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            // Get a local copy of the shared pointer to the job.
            local_job = m_current_job;
        }

        // Reset the flag. This worker now has the latest job.
        m_new_job_available = false;

        BlockHeader local_header = local_job->header;
        
        // Define the nonce range for this specific thread.
        uint64_t nonce_range_size = (uint64_t)UINT32_MAX / m_num_threads;
        uint32_t start_nonce = thread_id * nonce_range_size;
        uint32_t end_nonce = (thread_id == m_num_threads - 1) ? UINT32_MAX : start_nonce + nonce_range_size;

        Log::info("Thread " + std::to_string(thread_id) + " starting work on job " + std::to_string(local_job->job_id) +
                  " with nonce range " + std::to_string(start_nonce) + " - " + std::to_string(end_nonce));

        // The main hashing loop.
        for (uint32_t nonce = start_nonce; nonce < end_nonce; ++nonce) {
            // CRITICAL: Check if a new job has arrived. If so, stop this work immediately.
            if (m_new_job_available) {
                Log::warn("Thread " + std::to_string(thread_id) + " interrupting work for new job.");
                break; // Exit the for-loop to get the new job.
            }

            // If the whole miner is shutting down, exit completely.
            if (!m_is_running) break;

            local_header.nonce = nonce;
            hash32_t hash = double_sha256(&local_header, sizeof(BlockHeader));

            if (check_proof_of_work(hash, local_job->target)) {
                // We found a valid share!
                // The V2 submit_share call is much simpler.
                m_client->submit_share(local_job->job_id, nonce);
            }
        }
    }
    Log::info("Worker thread " + std::to_string(thread_id) + " finished.");
}