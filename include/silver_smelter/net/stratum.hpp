#pragma once

#include "silver_smelter/core/block.hpp"
#include <string>
#include <optional>

// Data received from the pool for a single mining job
struct StratumJob {
    std::string job_id;
    BlockHeader header;
    target_t target;
};

// MOCK Stratum Client: This is a placeholder for a real network client.
// It allows us to build and test the miner logic without a live connection.
class StratumClient {
public:
    StratumClient(const std::string& url, const std::string& user);

    bool connect();
    std::optional<StratumJob> get_job();
    void submit_share(const std::string& job_id, uint32_t nonce);

private:
    std::string m_url;
    std::string m_user;
};
