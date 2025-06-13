# Require CMake 3.15 or higher
cmake_minimum_required(VERSION 3.15)

# Define the project
project(Silver-Smelter VERSION 0.1.0 LANGUAGES CXX)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add the 'include' directory to the include path
include_directories(include)

# Find necessary libraries
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
# -- ADD THIS --
find_package(Boost REQUIRED COMPONENTS system thread)

# Define the executable target and its source files
add_executable(silver_smelter
    src/main.cpp
    src/core/block.cpp
    src/crypto/sha256.cpp
    src/miner/worker.cpp
    src/net/stratum.cpp
    src/util/log.cpp
)

# Link libraries to the executable
target_link_libraries(silver_smelter PRIVATE
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
    # -- AND ADD THIS --
    Boost::system
    Boost::thread
)

# --- For a nice development experience ---
# Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# Expose header files to VS Code for better IntelliSense
target_include_directories(silver_smelter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
