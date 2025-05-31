# PBX Telephone Exchange Simulator

This project implements a multithreaded server that simulates a simplified Private Branch Exchange (PBX) system. Each client connects over a TCP socket and behaves like a Telephone Unit (TU), capable of calling other extensions and simulating basic telephony operations.

## Features

- Multithreaded TCP server
- Each client is registered as a TU with a unique extension
- Supports basic telephony commands:
  - `pickup`: Lift receiver
  - `hangup`: Hang up call
  - `dial <ext>`: Call another TU
  - `chat <message>`: Send a message to the connected peer
- Signal-safe shutdown via `SIGHUP`
- Thread-safe synchronization using semaphores

## File Overview

| File         | Description |
|--------------|-------------|
| `main.c`     | Entry point; sets up the server and listens for connections |
| `server.c`   | Handles individual client interactions |
| `pbx.c`      | Manages PBX registry and extension mappings |
| `tu.c`       | Simulates telephone unit state transitions and messaging |
| `globals.c`  | Defines global symbols, including PBX instance |
| `csapp.c`    | Robust wrappers for system/network calls from CS:APP3e |
| `Makefile`   | Defines build targets for the project |

## Building

To compile the project:

`bash
make
`

To clean up compiled binaries:

`bash
make clean
`

## Running

Run the server by specifying the port number:

`bash
./pbx -p <port>
`

Example:

`bash
./pbx -p 8000
`

This starts the server listening on port 8000.

## Connecting Clients

Use `telnet` or `nc` to connect as a client:

`bash
nc localhost 8000
`

### Supported Commands (terminated with `\r\n`):

- `pickup`
- `hangup`
- `dial <extension>`
- `chat <message>`

Each command should be followed by a carriage return and newline (`\r\n`).

## Graceful Shutdown

To shut down the server, send a `SIGHUP`:

`bash
kill -SIGHUP <pid_of_server>
`

This triggers cleanup of all active TUs and extensions.

## Notes
- The server automatically detaches threads and handles memory cleanup on disconnects.
- Refer to the CS:APP textbook for implementation inspiration, particularly chapters on concurrency and networking. `csapp.c` is taken from this textbook. 
