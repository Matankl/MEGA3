# Reliable User Datagram Protocol (RUDP) and Transmission Control Protocol (TCP) File Transfer

## Overview

This project implements a file transfer system using both RUDP (Reliable User Datagram Protocol) and TCP (Transmission Control Protocol). It provides two methods for sending and receiving files: a reliable, custom-built UDP-based protocol (RUDP) and the standard TCP protocol. The project includes both sender and receiver implementations for each protocol.

### Components

- **RUDP**
  - `RUDP_API.c`: Implements the basic RUDP functionalities such as socket creation, data sending, and receiving.
  - `RUDP_Sender.c`: Implements the file sending process over RUDP, including data fragmentation, retries, and statistics collection.
  - `RUDP_Receiver.c`: Implements the file receiving process over RUDP, including checksum validation, acknowledgment handling, and statistics reporting.

- **TCP**
  - `TCP_Sender.c`: Implements the file sending process over TCP, including congestion control and statistics collection.
  - `TCP_Receiver.c`: Implements the file receiving process over TCP, including linked list management for tracking received data and calculating statistics.

### Features

- **Reliable File Transfer over UDP**: The RUDP protocol is implemented to ensure reliable data transfer using custom acknowledgment, timeout, and retransmission mechanisms.
- **File Transfer over TCP**: Standard TCP protocol is used for file transfer with optional congestion control algorithms (Reno, Cubic).
- **Statistics Collection**: The system collects and prints detailed statistics, including time taken, bandwidth, and average speed for each file transfer run.

## Getting Started

### Prerequisites

- GCC (GNU Compiler Collection) or any compatible C compiler.
- Linux/Unix environment (recommended for socket operations).

### Compiling the Project

Use the provided `Makefile` to compile the project. Run the following command in the terminal:

```sh
make all
```

This will compile all the necessary files and generate the executable binaries.

### Running the Project

#### RUDP Mode

1. **Receiver**: Start the RUDP receiver to listen for incoming connections:
    ```sh
    ./RUDP_Receiver <port>
    ```

2. **Sender**: Start the RUDP sender to send a file:
    ```sh
    ./RUDP_Sender <receiver_ip> <receiver_port>
    ```

#### TCP Mode

1. **Receiver**: Start the TCP receiver to listen for incoming connections:
    ```sh
    ./TCP_Receiver <port>
    ```

2. **Sender**: Start the TCP sender to send a file:
    ```sh
    ./TCP_Sender <server_ip> <server_port> <congestion_control>
    ```

   Replace `<congestion_control>` with either `reno` or `cubic` to specify the congestion control algorithm.

### File Transfer Workflow

- The sender generates random data of a specified size and sends it to the receiver.
- The receiver stores the received data, calculates the transfer time, and reports the statistics.
- After each transfer, the user can choose to resend the file or terminate the connection.

### Example Usage

```sh
# Example RUDP Receiver
./RUDP_Receiver 12345

# Example RUDP Sender
./RUDP_Sender 192.168.1.100 12345

# Example TCP Receiver
./TCP_Receiver 12345

# Example TCP Sender
./TCP_Sender 192.168.1.100 12345 cubic
```

## Project Structure

- **`RUDP_API.c`**: Implements core RUDP functionalities.
- **`RUDP_Sender.c`**: Handles the sending process over RUDP.
- **`RUDP_Receiver.c`**: Manages the receiving process over RUDP.
- **`TCP_Sender.c`**: Implements TCP sending with optional congestion control.
- **`TCP_Receiver.c`**: Handles the receiving process over TCP and manages transfer statistics.

## Contributing

If you would like to contribute to this project, please fork the repository and submit a pull request. For major changes, please open an issue to discuss what you would like to change.

