# Message Manager TCP-UDP

## Project Overview

This project implements a client-server architecture for a message broker platform that facilitates communication between UDP clients and TCP subscribers. The server acts as an intermediary, receiving messages from UDP clients and routing them to appropriate TCP subscribers based on topic subscriptions.

## Components

- **Server**: Acts as the broker between UDP clients and TCP subscribers
- **TCP Subscribers**: Connect to the server, subscribe to topics, and receive relevant messages
- **UDP Clients**: Publish messages on various topics (provided externally)

## Key Features

- **Dual Protocol Support**: Handles both TCP and UDP connections simultaneously
- **Topic-Based Message Routing**: Forwards messages to subscribers based on their topic interests
- **Wildcard Subscription Support**: 
  - `+` wildcard matches exactly one level in a topic path
  - `*` wildcard matches multiple levels in a topic path
- **Multiple Data Type Handling**: Supports INT, SHORT_REAL, FLOAT, and STRING message formats
- **Client State Persistence**: Maintains client subscriptions across disconnects and reconnects
- **Efficient Network Usage**: Only sends messages to interested subscribers
- **TCP Message Framing**: Custom protocol for reliable message delivery

## Implementation Details

### Data Structures

The implementation uses three primary structures:

1. **chat_packet**:
   - Used for TCP communication between server and subscribers

2. **Message**:
   - Format for messages received from UDP clients
   - Contains topic, data type, and content fields

3. **tcp_clients**:
   - Tracks TCP client information
   - Stores socket descriptor, active status, subscribed topics, and client ID

### Server Features

- Manages TCP and UDP connections on a specified port
- Processes subscribe/unsubscribe commands
- Routes UDP messages to interested TCP subscribers
- Handles client connections and reconnections
- Implements wildcard topic matching

The server maintains a list of all connected TCP clients and their topic subscriptions. When a UDP message arrives, the server determines which subscribers should receive the message based on their topic subscriptions and forwards the message accordingly.

### TCP Subscriber Features

- Connects to server with unique client ID
- Subscribes/unsubscribes to topics
- Parses and displays different message types

### Protocol Details

#### Message Format from UDP Clients

| Field      | Size            | Description                                                      |
|------------|-----------------|------------------------------------------------------------------|
| Topic      | 50 bytes        | String (max 50 chars) terminated with \0                         |
| Data Type  | 1 byte          | Unsigned int identifying the payload type (0-3)                  |
| Content    | Up to 1500 bytes| Format varies based on data type                                 |

## Wildcard Subscription

The server supports sophisticated topic matching with wildcards:

- **'+' Wildcard**: Matches exactly one level in a topic path
  - Example: `sensors/+/temperature` matches `sensors/living_room/temperature` but not `sensors/living_room/kitchen/temperature`

- **'*' Wildcard**: Matches multiple levels in a topic path
  - Example: `sensors/*` matches both `sensors/temperature` and `sensors/living_room/temperature`

The wildcard matching is implemented in the `wild_card()` function, which:
1. Tokenizes the topic path into levels
2. Compares each level with the pattern, handling special wildcard characters
3. Returns true if the topic matches the subscription pattern

## Usage

### Server

```bash
./server <PORT>
```

Commands:
- `exit` - Shut down the server and close all connections

### TCP Subscriber

```bash
./subscriber <CLIENT_ID> <SERVER_IP> <SERVER_PORT>
```

Commands:
- `subscribe <TOPIC>` - Subscribe to a topic (wildcards supported)
- `unsubscribe <TOPIC>` - Unsubscribe from a topic
- `exit` - Disconnect from the server

## Technical Challenges and Solutions

### Challenge: TCP Stream Handling

**Problem**: TCP is stream-oriented and doesn't preserve message boundaries.

**Solution**: Implemented a custom framing protocol with message length prefixes and used the `recv_all` and `send_all` functions to ensure complete message transmission.

### Challenge: Efficient Topic Matching

**Problem**: Wildcard patterns require sophisticated matching logic.

**Solution**: Developed a tokenized parsing algorithm that handles both '+' and '*' wildcards with proper level-by-level matching.

### Challenge: Client Reconnection

**Problem**: Maintaining client state across disconnections.

**Solution**: Client subscriptions are associated with client IDs rather than connections, allowing seamless reconnection with subscription preservation.

## Requirements

- C++ compiler with C++11 support
- POSIX-compliant operating system (Linux/Unix)
- Network socket libraries

## License

Copyright © 2023-2024 Dragan Dragos Ovidiu
