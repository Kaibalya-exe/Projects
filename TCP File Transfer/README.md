# TCP File Transfer Using C++

A client-server file transfer application built using C++ and TCP socket programming on Linux.

## Overview

This project explores network communication between a client and a server using TCP sockets. It demonstrates how two programs can establish a connection and transfer files over a network.

The project was developed using C++ in an Ubuntu environment through Windows Subsystem for Linux (WSL).

## Features

- TCP socket creation and communication.
- Client-server architecture.
- Server-side binding, listening, and connection acceptance.
- Client-side connection establishment.
- File handling using C++ file streams.
- Sending and receiving file data through sockets.
- Handling partial data transfers using a `send_all()` function.

## Technologies Used

- **Language:** C++
- **Networking:** TCP/IP, Linux Socket API
- **Environment:** Ubuntu on WSL
- **Libraries:** `iostream`, `fstream`, `sys/socket.h`, `unistd.h`, `netinet/in.h`, `arpa/inet.h`, `cstring`

## Project Structure

```text
TCP-File-Transfer/
│
├── client.cpp
├── server.cpp
└── README.md
```

## How It Works

1. The server creates a TCP socket.
2. The server binds the socket to an address and port.
3. The server listens for incoming connections.
4. The client creates a socket and connects to the server.
5. The client initiates the file transfer.
6. The file data is transmitted through the established TCP connection.
7. The server receives the transmitted data.

## Running the Project

Compile the server and client separately:

```bash
g++ server.cpp -o server
g++ client.cpp -o client
```

Start the server in one terminal:

```bash
./server
```

Run the client in another terminal:

```bash
./client
```

Follow the prompts to initiate the file transfer.

## Key Learnings

Through this project, I explored:

- How TCP connections work.
- The fundamentals of socket programming.
- Client-server communication.
- File handling in C++.
- The purpose of `bind()`, `listen()`, `accept()`, and `connect()`.
- How `send()` and `recv()` work.
- Why handling partial data transfers is important.

## Future Improvements

- Add support for transferring multiple files.
- Improve error handling and connection management.
- Implement transfer progress tracking.
- Explore secure file transfer mechanisms.

---

*Built as a hands-on project to understand computer networks, TCP communication, and system-level programming using C++.*