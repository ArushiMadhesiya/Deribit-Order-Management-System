# Deribit Order Management System

This project is a C++ application designed to interact with the [Deribit cryptocurrency exchange](https://www.deribit.com/). It provides functionalities for placing, canceling, and modifying orders, as well as retrieving order book data and viewing current positions. The system utilizes WebSocket communication for real-time data exchange and is optimized for performance, including latency measurement and asynchronous network communication.

## Features

- **Order Placement**: Place buy or sell orders with specified parameters.
- **Order Management**: Edit or cancel existing orders.
- **Order Book Retrieval**: Fetch the order book for specific instruments.
- **User Positions**: Retrieve detailed information about current positions.
- **Latency Measurement**: Measure and log latency for order placements to evaluate performance.

## Prerequisites

- **C++ Compiler**: Ensure you have a C++17 compatible compiler installed.
- **CMake**: Build system generator to manage the build process.
- **Boost Libraries**: Utilized for various functionalities, including asynchronous networking.

## Installation

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/ArushiMadhesiya/Deribit-Order-Management-System.git
   cd Deribit-Order-Management-System
   ```

2. **Build the Application**:
   ```bash
   mkdir build
   cd build
   cmake --build .
   ```

   This will generate the executable in the `build` directory.

## Usage

1. **Configuration**:
   - Before running the application, configure your API credentials and settings in the `config.json` file located in the root directory. Ensure you have valid API keys from Deribit.

2. **Running the Application**:
   ```bash
   ./GoQuant
   ```

   The application provides a command-line interface where you can input commands to interact with the Deribit exchange.
