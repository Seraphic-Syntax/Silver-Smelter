// New File: include/silver_smelter/net/v2_protocol.hpp
#pragma once

#include <cstdint>

// This ensures our structs are packed byte-for-byte, matching the protocol.
#pragma pack(push, 1)

// Every Stratum V2 message starts with this standard header.
struct MessageHeader {
    uint8_t  protocol;    // 0x02 for Stratum V2
    uint8_t  msg_type;
    uint16_t msg_len;     // Length of the message payload (following this header)
};

// --- Client to Server Messages ---

struct Subscribe {
    // Header: msg_type = 0
    char     pool_public_key[32];
    char     user_agent[32];      // Padded with nulls
    char     user_identity[32];   // Padded with nulls
    uint32_t max_extranonce_size;
};

struct SubmitShares {
    // Header: msg_type = 6
    uint32_t session_id;
    uint32_t job_id;
    uint32_t nonce;
    // Followed by user-defined extranonce data, if any.
};

// --- Server to Client Messages ---

// Message type constants from the server
constexpr uint8_t SETUP_CONNECTION_SUCCESS = 1;
constexpr uint8_t NEW_MINING_JOB = 100;

struct SetupConnectionSuccess {
    // Header: msg_type = 1
    uint32_t session_id;
    // Followed by other fields we can ignore for now
};

struct NewMiningJob {
    // Header: msg_type = 100
    uint32_t job_id;
    uint8_t  future_job; // 0 = standard job, 1 = future job
    uint32_t version;
    uint32_t bits;
    uint8_t  prev_block_hash[32];
    uint8_t  coinbase_tx_prefix[32]; // First part of the coinbase tx
    uint8_t  coinbase_tx_suffix[32]; // Second part of the coinbase tx
    // Followed by Merkle branch hashes
};

#pragma pack(pop)