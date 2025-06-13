#include "silver_smelter/util/log.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>

// Initialize the static mutex
std::mutex Log::m_mutex;

// ANSI color codes
const char* RESET_COLOR = "\033[0m";
const char* INFO_COLOR = "\033[34m"; // Blue
const char* WARN_COLOR = "\033[33m"; // Yellow
const char* ERROR_COLOR = "\033[31m"; // Red
const char* SUCCESS_COLOR = "\033[32m"; // Green

void Log::print(const std::string& level, const std::string& message, const char* color_code) {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << color_code
              << "[" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X") << "]"
              << "[" << level << "]: "
              << message
              << RESET_COLOR << std::endl;
}

void Log::info(const std::string& message) {
    print("INFO ", message, INFO_COLOR);
}

void Log::warn(const std::string& message) {
    print("WARN ", message, WARN_COLOR);
}

void Log::error(const std::string& message) {
    print("ERROR", message, ERROR_COLOR);
}

void Log::success(const std::string& message) {
    print("SUCCESS", message, SUCCESS_COLOR);
}
