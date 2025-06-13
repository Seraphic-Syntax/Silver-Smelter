Silver-Smelter 

<p align="center">
  <img src="https://www.deviantart.com/stash/0ydjy5jvqr4" alt="Silver-Smelter Logo" width="200"/>
</p>

<p align="center">
  <em>A high-performance, cross-platform Bitcoin mining client built from scratch in modern C++.</em>
</p>

<p align="center">
    <a href="https://github.com/Seraphic-Syntax/Silver-Smelter/blob/main/LICENSE"><img src="https://img.shields.io/badge/license-MIT-blue.svg" alt="License"></a>
    <img src="https://img.shields.io/badge/C%2B%2B-17-brightgreen.svg" alt="C++17">
    <img src="https://img.shields.io/badge/Stratum-V2-orange.svg" alt="Stratum V2">
</p>

---

> "The journey of a thousand hashes begins with a single line of code."

**Silver-Smelter** is an educational and functional implementation of a Bitcoin mining application. This project was built to explore the low-level mechanics of cryptocurrency mining, including cryptography, asynchronous networking, and multi-threaded performance optimization. It connects to mining pools using the modern and efficient **Stratum V2 binary protocol**.

## Features

- **Stratum V2 Protocol:** Natively implements the efficient binary protocol for communication with modern mining pools like Braiins Pool.
- **Modern C++ (17):** Built using modern C++ features for performance, safety, and readability.
- **Asynchronous Networking:** Utilizes **Boost.Asio** for high-throughput, non-blocking network I/O to handle communication with the pool without slowing down hashing.
- **Multi-threaded Hashing:** Spawns multiple worker threads to take full advantage of multi-core CPU architectures.
- **Cross-Platform Build System:** Uses **CMake** for a consistent and reliable build process on Linux, macOS, and Windows (via WSL).
- **Clean, Modular Architecture:** The codebase is organized by feature (crypto, net, miner) for easy navigation and maintenance.

## Building from Source

This project uses CMake. Ensure you have a C++17 compliant compiler (GCC, Clang), CMake, Git, and the Boost & OpenSSL development libraries installed.

**On Debian/Ubuntu/WSL:**
```bash
sudo apt update
sudo apt install build-essential cmake git libboost-all-dev libssl-dev

Build Steps:

Clone the repository:

BASH

git clone https://github.com/Seraphic-Syntax/Silver-Smelter.git
cd Silver-Smelter
Configure CMake (Release build for performance):

BASH

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
Compile the project:

BASH

cmake --build build -j
The final executable will be located at ./build/silver_smelter.

Usage
The miner is configured via command-line arguments. For a real-world scenario, you would implement argument parsing. For now, connection details are set in src/main.cpp.

BASH

# Run the compiled miner
./build/silver_smelter

Example Output:

PLAINTEXT

Collapse
[INFO]: Silver-Smelter Bitcoin Miner starting...
[INFO]: Pool: v2.us-east.stratum.braiins.com:3334
[INFO]: User: Seraphic-Syntax.worker1
[INFO]: Network thread started.
[INFO]: Miner configured to use 8 worker threads.
[INFO]: All miner threads started.
[INFO]: Miner is running. Press Enter to stop.
[INFO]: Resolving v2.us-east.stratum.braiins.com:3334...
[SUCCESS]: Connection established to 172.65.207.99!
[INFO]: Sending Subscribe message...
[INFO]: Dispatching message of type: 1
[SUCCESS]: Stratum V2 connection successful! Session ID: 123456789
[INFO]: Dispatching message of type: 100
[SUCCESS]: Received new V2 mining job ID: 98765
[SUCCESS]: New V2 job received by Miner: 98765
[INFO]: Worker thread 0 starting work on job 98765...
[INFO]: Worker thread 1 starting work on job 98765...
...

(Architecture)
The project is designed with a clear separation of concerns:

src/net/: Handles all network communication using the Stratum V2 protocol.
src/crypto/: Contains cryptographic primitives, primarily SHA-256.
src/core/: Defines core data structures like BlockHeader and target.
src/miner/: Manages worker threads, job synchronization, and the main hashing loop.
src/util/: Provides utility functions like thread-safe logging.
This modularity allows for easier testing and future feature development, such as implementing optimized hashing algorithms (e.g., AVX2) or supporting different mining protocols.

License
This project is licensed under the MIT License. See the LICENSE file for details.


