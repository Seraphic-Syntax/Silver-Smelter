#pragma once

#include "silver_smelter/crypto/sha256.hpp"
#include <cstdint>
#include <string>

// Bitcoin block header is 80 bytes.
// This structure must be packed to ensure correct byte ordering for hashing.
#pragma pack(push, 1)
struct BlockHeader {
    int32_t version;
    hash32_t prev_block_hash;
    hash32_t merkle_root;
    uint32_t timestamp;
    uint32_t bits;
    uint32_t nonce;
};
#pragma pack(pop)

// Represents the 256-bit difficulty target.
using target_t = std::array<uint8_t, 32>;

// Calculates the difficulty target from the compact 'bits' format.
target_t calculate_target_from_bits(uint32_t bits);

// Checks if a block header's hash meets the required difficulty target.
bool check_proof_of_work(const hash32_t& hash, const target_t& target);