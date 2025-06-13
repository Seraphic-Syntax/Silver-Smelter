#include "silver_smelter/crypto/sha256.hpp"
#include <openssl/sha.h>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <algorithm> // For std::reverse

hash32_t sha256(const void* data, size_t len) {
    hash32_t digest;
    SHA256_CTX ctx;
    if (!SHA256_Init(&ctx)) {
        throw std::runtime_error("Failed to initialize SHA256 context");
    }
    if (!SHA256_Update(&ctx, data, len)) {
        throw std::runtime_error("Failed to update SHA256 context");
    }
    if (!SHA256_Final(digest.data(), &ctx)) {
        throw std::runtime_error("Failed to finalize SHA256 context");
    }
    return digest;
}

hash32_t double_sha256(const void* data, size_t len) {
    hash32_t first_hash = sha256(data, len);
    return sha256(first_hash.data(), first_hash.size());
}

std::string hash_to_hex(const hash32_t& hash) {
    std::stringstream ss;
    // Bitcoin hashes are often displayed in reverse byte order.
    // We'll create a copy and reverse it for display purposes.
    hash32_t display_hash = hash;
    std::reverse(display_hash.begin(), display_hash.end());

    ss << std::hex << std::setfill('0');
    for (const auto& byte : display_hash) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}