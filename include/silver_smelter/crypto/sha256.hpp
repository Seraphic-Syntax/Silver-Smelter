#pragma once

#include <vector>
#include <array>
#include <cstdint>

// A 32-byte (256-bit) hash digest
using hash32_t = std::array<uint8_t, 32>;

// Calculates a single SHA256 hash.
hash32_t sha256(const void* data, size_t len);

// Calculates a double SHA256 hash (SHA256(SHA256(data))), as used in Bitcoin.
hash32_t double_sha256(const void* data, size_t len);

// Helper to convert hash to a hex string for printing
std::string hash_to_hex(const hash32_t& hash);