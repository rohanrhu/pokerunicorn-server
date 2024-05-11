# PokerUnicorn Server (pkrsrv)

PokerUnicorn Server is a poker server software written in C for the PokerUnicorn Infrastructure project.

[Play the demo on web](https://meowingcat.io/projects/poker/play/) (Uses Monero Stagenet. No real money.)

## Important Legal Notice

This is a gambling service software and it is not allowed to use this software in countries where gambling is illegal.

This software is an open source project and it is not allowed to use this software for commercial purposes. (Read the license.)

This poker server and its client are not using any real money or real blockchain. It is using Monero Stagenet which is a testing purposed blockchain network and it doesn't have any real value.

If you are interested in using this software for commercial purposes, you can contact me for a special license and consulting.

## ❤️ Donate

If my technology is useful for your commercial purposes or you just love it, donate me some cute amount of USDT/USDC, Bitcoin, or Monero;
**developing and maintaining this software is not easy and it takes a lot of time and effort.**

| Currency          | Address                                                                                         |
| ----------------- | ----------------------------------------------------------------------------------------------- |
| BTC               | bc1qhvlc762kwuzeawedl9a8z0duhs8449nwwc35e2                                                      |
| ETH / USDT / USDC | 0x1D99B2a2D85C34d478dD8519792e82B18f861974                                                      |
| XMR               | 8A8HWvGp8d3Qf8Qn1ei97RXPJiQYodvBxLSaPbnLEKAZ2pjnAbmNLn59HNPKirrAmKTfNEfjbobdi33zV1CwDQ7qRLxiZvR |

### Patreon

You can donate me on Patreon too:

[![Support me on Patreon](https://img.shields.io/endpoint.svg?url=https%3A%2F%2Fshieldsio-patreon.vercel.app%2Fapi%3Fusername%3DEvrenselKisilik%26type%3Dpatrons&style=flat-square)](https://patreon.com/EvrenselKisilik)

### GitHub Sponsors

You can donate me on GitHub Sponsors too:

[![Support me on GitHub Sponsors](https://img.shields.io/github/sponsors/rohanrhu?style=flat-square&label=GitHub%20Sponsors)](https://github.com/rohanrhu)

## How does it work?

Here is a short explanation of how the server works:

* Whole server is written in C and it is designed to be fast and efficient.
* Lobby module (`pkrsrv_lobby_t`) is the main module that manages the game sessions and clients and it works with an event loop. (`pkrsrv_event_loop_t* pkrsrv_lobby_t*::eventloop`)
* Memory management and thread-safety are done with reference counting and mutex locks.
  * The game server uses reference-counting for HEAP objects and it is thread-safe with per-object mutex locks.
  * Most of HEAP objects are reference-counted.
* The TCP protcol is a custom stream protocol that is designed for the PokerUnicorn Infrastructure project. (See [PROTOCOL.md](PROTOCOL.md).)
  * Designed to be efficient and fast.
  * Uses SSL for security.
  * It has WebSocket support. (The WebSocket feature supports WS fragmentation too.)
* The server uses PostgreSQL as database.
* Whole server is Dockerized and it is easy to develop and deploy.

## Containerization for dev environment

To enable using GDBFrontend and directly developing on Linux which is the server is supposed to work on,
we have a Docker setup.

The Docker composition has following things:

* Game Server Container
  * GNU Toolchain to build the server (GCC, GNU Make, GDB, ...)
  * [GDBFrontend](https://github.com/rohanrhu/gdb-frontend) (Latest Git snapshot)
  * Valgrind
* Postgres Container
* pgAdmin4 Container

### Running everything at once

To start everything, just do this:

```bash
docker compose up -d
```

### Setting SSL certificate

* Place SSL certificate and key files into `PokerServer/host`. (You can do that on host machine, it is mounted to container.)
* Create a `.env` file in `PokerServer` folder and specify SSL certificate and key files:

```bash
PKRSRV_SSL_KEY_PATH=/root/PokerServer/host/ssl.key
PKRSRV_SSL_CER_PATH=/root/PokerServer/host/ssl.crt
```

### Setting Bind Address

You can set bind address with the environmant variable `PKRSRV_GDBFRONTEND_BIND_ADDRESS` in `.env` file.

```bash
PKRSRV_GDBFRONTEND_BIND_ADDRESS=0.0.0.0
```

> **Note**
> Server folder is placed at `/root/PokerServer` in container.
> Better you specify absolute paths for environment variables.

### Using the GDBFrontend session

When the container is up, GDBFrontend is reachable at:

```text
http://127.0.0.1:5550
```

## Data Structure Labels

| Kind / Label | Feature                              |
| ------------ | ------------------------------------ |
| Serializable | Already serializable representations |
| To-serialize | Will be serialized for something     |

## Parallelism and Thread-Safety

Each client has their own threads and the rest of the server is using event loop (`pkrsrv_eventloop_t* pkrsrv_lobby_t*::eventloop`).

```text
+-------------------------------------------+    +-----------------------------------------+
|                                           |    |                                         |
|               Game Sessions               |    |             Client Sockets              |
|     (Multi-threaded per game session)     |    |            (Multi-Threaded)             |
|                                           |    |                                         |
| +---------------------------------------+ |    |  +-----------------------------------+  |
| |                                       | |    |  |                                   |  |
| |             Game Session              | |    |  |          Client Thread            |  |
| |                                       | |    |  |                                   |  |
| |                                       | |    |  |       +-------------------+       |  |
| |                                       | |    |  |       |   Client Socket   |       |  |
| |  +---------------------------------+  | |    |  |       +-------------------+       |  |
| |  |                            +------------------------+                            |  |
| |  |                            |       Mutex Lock       |----------------------------+  |
| |  |                            +------------------------+                               |
| |  |                                 |  | |    |  +-----------------------------------+  |
| |  |                                 |  | |    |  |                                   |  |
| |  |                                 |  | |    |  |          Client Thread            |  |
| |  |          Poker Machine          |  | |    |  |                                   |  |
| |  |                                 |  | |+------------> +-------------------+       |  |
| |  |                                 |  | ||+------------ |   Client Socket   |       |  |
| |  |                                 |  | |||  |  |       +-------^---|-------+       |  |
| |  |                                 |  | |||  |  |               |   |               |  |
| |  |                                 |  | |||  |  +---------------|---|---------------+  |
| |  +--------------^---|--------------+  | |||  |                  * * *                  |
| +-----------------|---|-----------------+ |||  +------------------|---|------------------+
|                   * * *                   |||  +------------------|---⌄------------------+
|                   |   |                   |||  |      +---------------------------+      |
|    +--------------|---⌄--------------+    |||  |      |      GDScript Client      |      |
|    |     +---------------------+ >---|----|+|  |      +---------------------------+      |
|    |     |      Event Loop     | <---|----|-+  |                                         |
|    |     +---------------------+     |    |    |                                         |
|    |                                 |    |    |              Game Client                |
|    |             Lobby               |    |    |      [SynchronouusWebSocketStream]      |
|    |    (Implements Interactions)    |    |    |               [BigCat]                  |
|    |                                 |    |    |                                         |
|    +---------------------------------+    |    |                                         |
|                                           |    |                                         |
+-------------------------------------------+    +-----------------------------------------+

```

## Contributing

You can contribute to this project by forking this repository and sending pull requests. You can also open issues for bugs and feature requests.
Or you can donate me to support the development of this project.

### Things to do when contributing to the code

* Please follow the coding style of the project. It is important for being a code pawing cat issueless.
* Always run the app on GDBFrontend (it is default) and use assertions (`PKRSRV_UTIL_ASSERT(...)`) and have a breakpoint at `pkrsrv_util_assert_fail()`.
* Always run your app with Valgrind and ensure there is no memory leak before sending a pull request.

## [Protocol Documentation](PROTOCOL.md)

You can read the network protocol documentation from [PROTOCOL.md](PROTOCOL.md).

## Identifiers

Runtime identifier module is `pkrsrv_uniqid`.

An identifer is an unsigned 64 bit scalar of the type `pkrsrv_uniqid_uuid_t` which is increased by `+1` for each generation.

## Sessions Storage Architecture

Sessions are HEAP objects of `pkrsrv_session_t*`.

* Linked-list per lobby: `lobby->sessions` of `pkrsrv_lobby_session_t*`
* Trie map with ASCII number range [48-57] (`trie.c` / `pkrsrv_trie_...__index`)
* Linked-list per client (`pkrsrv_lobby_client_t`) points to `pkrsrv_lobby_session_t*` through `pkrsrv_lobby_client_session_t::session`.

## Clients Storage Architecture

### Clients

Clients are HEAP objects of `pkrsrv_server_client_t*`.

* Linked-list per server: `server->clients` of `pkrsrv_server_client_t*`
* Trie map with ASCII number range [48-57] (`trie.c` / `pkrsrv_trie_...__index`)

### Lobby Clients

Lobby clients are HEAP objects of `pkrsrv_lobby_client_t*`.

* Linked-list per server client (`pkrsrv_server_client_t`) points to `pkrsrv_lobby_client_t*` through `pkrsrv_server_client_t::owner`.
* Linked-list per session (`pkrsrv_lobby_session_t`) of `pkrsrv_lobby_session_client_t*`

## License

GNU General Public License v3

Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> ([https://meowingcat.io](https://meowingcat.io))

GNU General Public License v3 (GPL-3)

You may copy, distribute and modify the software as long as you track changes/dates in source files. Any modifications to or software including (via compiler) GPL-licensed code must also be made available under the GPL along with build & install instructions.
