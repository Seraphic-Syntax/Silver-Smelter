#include "silver_smelter/net/stratum.hpp"
#include "silver_smelter/util/log.hpp"

StratumClient::StratumClient(const std::string& url, const std::string& user)
    : m_url(url), m_user(user) {}

bool StratumClient::connect() {
    // MOCK IMPLEMENTATION: In a real client, this would open a TCP socket.
    Log::info("MOCK: Connecting to Stratum server at " + m_url + "...");
    Log::info("MOCK: Subscribing with user: " + m_user);
    Log::success("MOCK: Connection and subscription successful.");
    return true;
}

std::optional<StratumJob> StratumClient::get_job() {
    // MOCK IMPLEMENTATION: Return a hardcoded, plausible-looking job.
    Log::info("MOCK: Requesting new job from pool...");
    StratumJob job;
    job.job_id = "job001";
    job.header.version = 0x20000000;
    // Example prev_block_hash (needs to be 32 bytes)
    std::fill(job.header.prev_block_hash.begin(), job.header.prev_block_hash.end(), 0xaa);
    // Example merkle_root
    std::fill(job.header.merkle_root.begin(), job.header.merkle_root.end(), 0xbb);
    job.header.timestamp = time(0);
    job.header.bits = 0x1d00ffff; // A reasonably easy difficulty for testing
    job.header.nonce = 0; // Miner will set this

    job.target = calculate_target_from_bits(job.header.bits);

    Log::info("MOCK: Received new job " + job.job_id);
    return job;
}

void StratumClient::submit_share(const std::string& job_id, uint32_t nonce) {
    // MOCK IMPLEMENTATION: Just log the submission.
    Log::success("MOCK: Submitting share for job " + job_id + " with nonce " + std::to_string(nonce));
}