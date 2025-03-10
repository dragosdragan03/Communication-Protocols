# Dataplane Router

## Overview

Dataplane Router is a network routing implementation in C/C++ that efficiently handles IPv4 packet forwarding with ARP (Address Resolution Protocol) and ICMP (Internet Control Message Protocol) functionality. This project demonstrates the core functionality of a network router's data plane - the component responsible for forwarding packets based on routing decisions.

## Features

- **Optimized Routing Table** with binary search implementation for fast route lookups
- **ARP Protocol Implementation** with dynamic request/reply management
- **IPv4 Packet Handling** with efficient forwarding mechanisms
- **ICMP Support** for handling network errors and echo requests
- **Packet Queuing System** to minimize packet loss during ARP resolution

## Implementation Details

### Data Structures

The router maintains several critical data structures:
- `rtable`: Routing table for making forwarding decisions
- `arp_table`: ARP table mapping IP addresses to MAC addresses
- Three queues for pending packets awaiting ARP resolution

### Key Components

#### Routing Logic

The routing table is optimized using a custom comparator function (`compare_table`) and quick sort to organize routes by prefix and mask. The `get_best_route` function implements binary search to efficiently find the optimal route for each packet, significantly reducing lookup times compared to linear searching.

#### ARP Protocol Management

The router implements a complete ARP cycle:
- `receive_arp_request`: Processes incoming ARP requests and generates appropriate replies
- `receive_arp_reply`: Updates the ARP table and forwards queued packets when MAC addresses are resolved
- `send_arp_request`: Generates ARP requests when a MAC address is needed but not found in the ARP table

#### ICMP Support

Error handling and network diagnostics are managed through ICMP:
- `icmp_message`: Creates ICMP packets for "Destination Unreachable" and "Time Exceeded" scenarios
- `icmp_echo_request`: Handles ICMP echo requests (ping) by generating echo replies

#### Packet Queue Management

To prevent packet loss while awaiting ARP resolution, the router implements a queuing system that:
- Stores packets that cannot be immediately forwarded due to missing MAC addresses
- Tracks the size and interface of each queued packet
- Processes the queue once ARP replies are received

## Performance Optimizations

- Binary search in the routing table reduces lookup complexity from O(n) to O(log n)
- Efficient ARP table management minimizes unnecessary network traffic
- Smart packet queuing improves throughput by preventing packet drops during ARP resolution
- Header manipulation is optimized to reduce memory operations

## Technical Details

The router handles various network scenarios including:
- Route lookup and packet forwarding
- ARP resolution for unknown MAC addresses
- ICMP error generation for unreachable destinations
- TTL (Time To Live) management with appropriate ICMP responses
- Packet queuing during the ARP resolution process

## Usage

To compile and run the Dataplane Router:

```bash
make
./router
```

## Requirements

- C/C++ compiler (GCC recommended)
- Development libraries for network packet manipulation

## License

Copyright © 2023-2024 Dragan Dragos Ovidiu
