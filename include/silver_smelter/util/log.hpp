#pragma once

#include <string>
#include <mutex>

// A simple, thread-safe, singleton logger
class Log {
public:
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);
    static void success(const std::string& message);

private:
    static void print(const std::string& level, const std::string& message, const char* color_code);
    static std::mutex m_mutex;
};