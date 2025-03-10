# Network and Systems Programming Projects

This repository contains three networking and systems programming projects developed in C/C++. Each project demonstrates different aspects of network communication, data handling, and protocol implementation.

## Projects Overview

<!-- ### 1. [Dataplane Router](./dataplane-router/) -->
### 1. [Dataplane Router](https://github.com/dragosdragan03/Communication-Protocols/tree/main/dataplane-router)

A network routing implementation that efficiently handles IPv4 packet forwarding with ARP and ICMP functionality. 

**Key Features:**
- Optimized routing table with binary search for fast lookups
- Complete ARP protocol implementation with request/reply cycle
- ICMP support for error handling and ping requests
- Packet queuing system to prevent packet loss during ARP resolution

**Technologies:** C/C++, IPv4, ARP, ICMP, Wireshark

### 2. [HTTP Client REST API](https://github.com/dragosdragan03/Communication-Protocols/tree/main/http-client-rest-api)

A command-line client that communicates with a RESTful API through HTTP messages for managing a digital library.

**Key Features:**
- User authentication (register/login)
- Library access management with tokens
- Book operations (view, add, delete)
- JSON-based communication
- Comprehensive error handling

**Technologies:** C/C++, HTTP, REST API, JSON

### 3. [Message Manager TCP-UDP](https://github.com/dragosdragan03/Communication-Protocols/tree/main/message-manager-tcp-udp)

A client-server message broker facilitating communication between UDP publishers and TCP subscribers.

**Key Features:**
- Topic-based message routing
- Wildcard subscription patterns (`+` and `*`)
- Multiple data type support (INT, SHORT_REAL, FLOAT, STRING)
- Client state persistence across reconnections
- Custom TCP message framing protocol

**Technologies:** C/C++, TCP, UDP, Socket Programming, Multi-client Server

## Author

Copyright © 2023-2024 Dragan Dragos Ovidiu