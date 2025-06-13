#include "silver_smelter/core/block.hpp"
#include <algorithm> // For std::reverse and std::equal

target_t calculate_target_from_bits(uint32_t bits) {
    // The 'bits' field is a compact representation of the target.
    // The first byte is the exponent, the next three are the coefficient.
    uint32_t exponent = bits >> 24;
    uint32_t coefficient = bits & 0x00FFFFFF;

    // Formula: target = coefficient * 2^(8 * (exponent - 3))
    // We construct the 256-bit target byte by byte.
    target_t target{}; // Initialize to all zeros
    int byte_pos = 32 - exponent;

    if (byte_pos >= 0 && byte_pos < 32) {
        target[byte_pos] = (coefficient >> 16) & 0xFF;
    }
    if (byte_pos + 1 >= 0 && byte_pos + 1 < 32) {
        target[byte_pos + 1] = (coefficient >> 8) & 0xFF;
    }
    if (byte_pos + 2 >= 0 && byte_pos + 2 < 32) {
        target[byte_pos + 2] = coefficient & 0xFF;
    }
    return target;
}

bool check_proof_of_work(const hash32_t& hash, const target_t& target) {
    // To be valid, the hash must be numerically less than or equal to the target.
    // Since both are treated as little-endian 256-bit numbers, we can do a
    // byte-by-byte comparison from most significant to least significant.
    // The byte arrays are little-endian, so we compare them in reverse.
    return std::lexicographical_compare(
        hash.rbegin(), hash.rend(),
        target.rbegin(), target.rend()
    );
}