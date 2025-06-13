# Silver-Smelter

A high-performance Bitcoin miner written in C++.

- **Author**: Seraphic-Syntax
- **Braiins Pool Username**: Seraphic-Syntax

## About

This project is an implementation of a Bitcoin mining client designed to connect to mining pools using the Stratum V2 protocol.

## Building the Project

This project uses CMake.

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/Seraphic-Syntax/Silver-Smelter.git
    cd Silver-Smelter
    ```

2.  **Configure CMake:**
    ```bash
    cmake -S . -B build
    ```

3.  **Build the executable:**
    ```bash
    cmake --build build
    ```

The final executable will be located at `build/silver_smelter`.

## Usage

```bash
./build/silver_smelter --url <pool_url> --user <your_username>