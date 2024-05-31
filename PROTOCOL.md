# PokerUnicorn Network Protocol

Currently the game server has a TCP server that works on SSL and supports WebSocket too.

> **Important**
> WebSocket mode works as a wrapper for the TCP protocol. The protocol flows in the same way in WebSocket mode too.
> In fact, WebSocket is a message-based protocol **not a streaming protocol** while TCP is a streaming protocol.
> A WS client that connects to the server must do buffered synchronous network reads on the protocol.
> I made a `SynchronousWebSocketStream` library for this purpose in the client side. The client uses that.

## TCP Server Protocol

The game server works on SSL and supports WebSocket too.

### Supported Protocols

* Pure TCP over TLS
* WebSocket over TLS

When the game server receives HTTP header for WebSocket handshaking, it switches to WebSocket mode for that client.

#### TLS Support

SSL key and certificate files are passed with `--ssl-key-file` and `--ssl-cert-file` CLI arguments.

#### WebSocket Support

WebSocket module is `pkrsrv_websocket_...`. The server understands and switches to WebSocket when client is a WebSocket client.

The protocol flows in the same way in WebSocket mode too.. WS client must do buffered synchronous network reads on the protocol.

### Frames and Network Commands

Specifications of TCP protocol and server module usage:

* Each scalar packet start with the header frame.
* Protocol object layouts **are not memory-padded** and named as `pkrsrv_server_packet_frame_[...]_t`.
* Server module passes network commands in `pkrsrv_server_packet_[...]_t`.

> **Note**
> Frames are not memory-padded! They must be packed.

#### Packet Structure

```text
| ----- | ---------------------------------- | ------------------------- |
|  Byte |              Data                  |         Description       |
| ----- | ---------------------------------- | ------------------------- |
|       |        (64-bit Header Frame)       |                           |
|    0. |              OPCODE                |                           |
|    1. |                                    |                           |
|    2. |                                    |                           |
|    3. |                                    |                           |
|    4. |              LENGTH                | Generic length for        |
|    5. |                                    | generic use cases         |
|    6. |                                    |                           |
|    7. |                                    |                           |
| ----- | ---------------------------------- | ------------------------- |
|    0. |           (Packet Frame)           | Packet frame that its     |
|    N. |                ...                 | length determined by      |
|       |                                    | the OPCODE                |
|       |                                    |                           |
|       |                                    | Packet frames contain     |
|       |                                    | Vector field lengths as   |
|       |                                    | scalar types              |
| ----- | ---------------------------------- | ------------------------- |
|    0. |             [Vector 0]             | Vector lengths are spec-  |
|    N. |                ...                 | ified by frame fields     |
| ----- | ---------------------------------- | ------------------------- |
|    0. |             [Vector N]             |                           |
|    N. |                ...                 |                           |
| ----- | ---------------------------------- | ------------------------- |
```

#### Header

| Field     | Type          | Size   | Description                                                     |
| --------- | ------------- | ------ | --------------------------------------------------------------- |
| `opcode`  | `uint32_t`    | 4      | Specifies how to marshal & send and receive & parse packets     |
| `length`  | `uint32_t`    | 4      | Generic length for some generic packet implementations          |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_header {
    uint32_t opcode;
    uint32_t length;
};
```

##### Opcodes

```c
enum PKRSRV_SERVER_OPCODE {
    PKRSRV_SERVER_OPCODE_NOP = 0,
    PKRSRV_SERVER_OPCODE_MEOW,
    PKRSRV_SERVER_OPCODE_PING,
    PKRSRV_SERVER_OPCODE_PONG,
    PKRSRV_SERVER_OPCODE_LOGIN,
    PKRSRV_SERVER_OPCODE_LOGIN_RES,
    PKRSRV_SERVER_OPCODE_SIGNUP,
    PKRSRV_SERVER_OPCODE_SIGNUP_RES,
    PKRSRV_SERVER_OPCODE_GET_ACCOUNT,
    PKRSRV_SERVER_OPCODE_ACCOUNT,
    PKRSRV_SERVER_OPCODE_ENTER,
    PKRSRV_SERVER_OPCODE_ENTER_RES,
    PKRSRV_SERVER_OPCODE_LEAVE,
    PKRSRV_SERVER_OPCODE_LEAVE_RES,
    PKRSRV_SERVER_OPCODE_JOIN,
    PKRSRV_SERVER_OPCODE_JOIN_RES,
    PKRSRV_SERVER_OPCODE_UNJOIN,
    PKRSRV_SERVER_OPCODE_UNJOIN_RES,
    PKRSRV_SERVER_OPCODE_POKER_INFO,
    PKRSRV_SERVER_OPCODE_POKER_STATE,
    PKRSRV_SERVER_OPCODE_POKER_ACTION,
    PKRSRV_SERVER_OPCODE_POKER_ACTION_REFLECTION,
    PKRSRV_SERVER_OPCODE_POKER_END,
    PKRSRV_SERVER_OPCODE_POKER_RESTARTED,
    PKRSRV_SERVER_OPCODE_JSON,
    PKRSRV_SERVER_OPCODE_UNJOINED,
    PKRSRV_SERVER_OPCODE_GET_TABLES,
    PKRSRV_SERVER_OPCODE_TABLES,
    PKRSRV_SERVER_OPCODE_GET_SESSIONS,
    PKRSRV_SERVER_OPCODE_SESSIONS,
    PKRSRV_SERVER_OPCODE_UPDATE_ACCOUNT,
    PKRSRV_SERVER_OPCODE_UPDATE_ACCOUNT_RES,
    PKRSRV_SERVER_OPCODE_SERVER_INFO,
    PKRSRV_SERVER_OPCODE_OVER_CAPACITY,
    PKRSRV_SERVER_OPCODE_END // Opcodes terminator
};
```

#### Meow

Client must send MEOW packet after connection is established to start using the protocol.

### Over Capacity

Sent when server is over capacity. Client must disconnect and try again later.

#### Signup

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `CLIENT_TO_SERVER`                                              |
| `has_response` | `true`                                                          |

Fields:

| Field              | Type             | Size   | Description                                                         |
| ------------------ | ---------------- | ------ | ------------------------------------------------------------------- |
| `id_token_length`     | `uint16_t`       | 2      | Length of id_token NULL-terminated string (includes NULL-char too)     |
| `password_length`  | `uint16_t`       | 2      | Length of password NULL-terminated string (includes NULL-char too)  |
| `id_token`            | `char*`          | ^      | (non-NULL-terminated) Being sent by the server (ptr)                |
| `password`         | `char*`          | ^      | (non-NULL-terminated) Being sent by the server (ptr)                |
| `name`             | `char*`          | ^      | (non-NULL-terminated) Being sent by the server (ptr)                |
| `avatar`           | `unsigned char*` | ^      | Binary representation of avatar image                               |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_signup {
    uint16_t id_token_length;
    uint16_t password_length;
    uint16_t name_length;
    uint32_t avatar_length;
};
```

Next to send:

| Field      | Type                | Size    | Description                                              |
| ---------- | ------------------- | ------- | -------------------------------------------------------- |
| `id_token`    | `char*`             | ^       | (non-NULL-terminated) Being sent by the server (ptr)     |
| `password` | `char*`             | ^       | (non-NULL-terminated) Being sent by the server (ptr)     |
| `name`     | `char*`             | ^       | (non-NULL-terminated) Being sent by the server (ptr)     |
| `avatar`   | `unsigned char*`    | ^       | Binary representation of avatar image                    |

#### Signup Response

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `SERVER_TO_CLIENT`                                              |

Fields:

| Field         | Type               | Size   | Description                                                     |
| --------------| ------------------ | ------ | --------------------------------------------------------------- |
| `is_ok`       | `uint8_t`    | 1      | Is there any error?                                             |
| `is_logined`  | `uint8_t`    | 1      | Was logged in after sign up?                                    |
| `status` | `uint16_t` (`pkrsrv_server_packet_signup_res_status_t`) | 2 | Status code                     |

Status Codes:

```c
typedef enum PKRSRV_SERVER_PACKET_SIGNUP_RES_STATUS {
    PKRSRV_SERVER_PACKET_SIGNUP_RES_STATUS_OK = 0,
    PKRSRV_SERVER_PACKET_SIGNUP_RES_STATUS_ERROR,
    PKRSRV_SERVER_PACKET_SIGNUP_RES_STATUS_ALREADY_EXISTS
} pkrsrv_server_packet_signup_res_status_t;
```

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_signup_res {
    unsigned char is_ok;
    unsigned char is_logined;
    uint16_t status;
};
```

Next to receive:

Next to receive if `is_ok && is_logined`:

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_signup_res_account {
    uint16_t id_token_length;
    uint16_t name_length;
    uint32_t avatar_length;
    uint16_t xmr_deposit_address_length;
    uint64_t id;
    uint64_t balance;
};
```

| Field              | Type               | Size   | Description                                     |
| ------------------ | ------------------ | ------ | ----------------------------------------------- |
| `id_token_length`  | `uint16_t`         | 2      | Length of id_token                              |
| `name_length`      | `uint16_t`         | 2      | Length of user name                             |
| `avatar_length`    | `uint16_t`         | 2      | Length of avatar binary                         |
| `xmr_deposit_address_length` | `uint16_t` | 2      | Length of XMR deposit address binary          |
| `id`               | `uint64_t`         | 8      | Account ID                                      |
| `balance   | `uint64_t`         | 8      | Account balance                                 |

Next to receive:

| Vector      | Type                             | Size               |
| ----------- | -------------------------------- | ------------------ |
| `id_token`     | `unsigned char[id_token_length]`    | `id_token_length`     |
| `name`      | `unsigned char[name_length]`     | `name_length`      |
| `avatar`    | `unsigned char[avatar_length]`   | `avatar_length`    |
| `xmr_deposit_address` | `unsigned char[xmr_deposit_address_length]` | `xmr_deposit_address_length` |

#### Login

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `CLIENT_TO_SERVER`                                              |
| `has_response` | `true`                                                          |

Fields:

| Field              | Type        | Size           | Description                                                         |
| ------------------ | ----------- | -------------- | ------------------------------------------------------------------- |
| `id_token_length`     | `uint16_t`  | 2              | Length of id_token NULL-terminated string (includes NULL-char too)     |
| `password_length`  | `uint16_t`  | 2              | Length of password NULL-terminated string (includes NULL-char too)  |
| `id_token`            | `char*`     | ^ | (non-NULL-terminated) Being sent by the server (ptr)                |
| `password`         | `char*`     | ^ | (non-NULL-terminated) Being sent by the server (ptr)                |

Next to send:

| Field      | Type    | Size              | Description                          |
| ---------- | ------- | ----------------- | ------------------------------------ |
| `id_token`    | `char*` | `id_token_length`    | ID Token to login (non-NULL-terminated)     |
| `password` | `char*` | `password_length` | Password to login (non-NULL-terminated)  |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_login {
    uint16_t id_token_length;
    uint16_t password_length;
    char* id_token;
    char* password;
};
```

##### Login Response

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `SERVER_TO_CLIENT`                                              |

Fields:

| Field         | Type               | Size   | Description                                                     |
| --------------| ------------------ | ------ | --------------------------------------------------------------- |
| `is_ok`       | `uint8_t`    | 1      | Is there any error?                                             |
| `is_logined`  | `uint8_t`    | 1      | Was logging in successful?                                      |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_login_res {
    unsigned char is_ok;
    unsigned char is_logined;
};
```

Next to receive if `is_ok && is_logined`:

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_login_res_account {
    uint16_t xmr_deposit_address_length;
    uint16_t id_token_length;
    uint16_t name_length;
    uint32_t avatar_length;
    uint64_t id;
    uint64_t balance;
};
```

| Field           | Type                   | Size   | Description                                  |
| --------------- | ---------------------- | ------ | -------------------------------------------- |
| `xm_deposit_address_length` | `uint16_t` | 2      | Length of XMR deposit address binary         |
| `id_token_length`  | `uint16_t`          | 2      | Length of id_token                           |
| `name_length`   | `uint16_t`             | 2      | Length of user name                          |
| `avatar_length` | `uint16_t`             | 2      | Length of avatar binary                      |
| `balance`       | `uint64_t`             | 8      | Account balance                              |

Next to receive:

| Vector      | Type                             | Size               |
| ----------- | -------------------------------- | ------------------ |
| `xmr_deposit_address` | `unsigned char[xmr_deposit_address_length]` | `xmr_deposit_address_length` |
| `id_token`     | `unsigned char[id_token_length]`    | `id_token_length`     |
| `name`      | `unsigned char[name_length]`     | `name_length`      |
| `avatar`    | `unsigned char[avatar_length]`   | `avatar_length`    |

#### Account

Sent to client when account is updated.

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `SERVER_TO_CLIENT`                                              |

Fields:

| Field         | Type               | Size   | Description                                                     |
| --------------| ------------------ | ------ | --------------------------------------------------------------- |
| `xm_deposit_address_length` | `uint16_t` | 2      | Length of XMR deposit address binary                           |
| `id_token_length`  | `uint16_t`         | 2      | Length of id_token                                                 |
| `name_length`   | `uint16_t`         | 2      | Length of user name                                             |
| `avatar_length` | `uint16_t`         | 2      | Length of avatar binary                                         |
| `id`           | `uint64_t`         | 8      | Account ID                                                     |
| `balance`           | `uint64_t`         | 8      | Account balance                                                     |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_account {
    uint16_t xmr_deposit_address_length;
    uint16_t id_token_length;
    uint16_t name_length;
    uint32_t avatar_length;
    uint64_t id;
    uint64_t balance;
};
```

#### Update Account

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `CLIENT_TO_SERVER`                                              |
| `has_response` | `true`                                                          |

| Field           | Type               | Size   | Description                                                     |
| --------------- | ------------------ | ------ | --------------------------------------------------------------- |
| `name_length`   | `uint16_t`         | 2      | Length of user name                                             |
| `avatar_length` | `uint32_t`         | 4      | Length of avatar binary                                         |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_update_account {
    uint16_t name_length;
    uint32_t avatar_length;
};
```

Next to send:

| Vector      | Type                             | Size               |
| ----------- | -------------------------------- | ------------------ |
| `name`      | `unsigned char[name_length]`     | `name_length`      |
| `avatar`    | `unsigned char[avatar_length]`   | `avatar_length`    |

##### Update Account Response

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `SERVER_TO_CLIENT`                                              |

Fields:

| Field         | Type               | Size   | Description                                                     |
| --------------| ------------------ | ------ | --------------------------------------------------------------- |
| `is_ok`       | `uint8_t`    | 1      | Is there any error?                                             |
| `is_avatar_too_big`  | `uint8_t`    | 1      | Was avatar binary too big?                               |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_login_res {
    unsigned char is_ok;
    unsigned char is_avatar_too_big;
};
```

#### Enter

Enters a table/session as a watcher.

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `CLIENT_TO_SERVER`                                              |
| `has_response` | `true`                                                          |

Fields:

| Field              | Type          | Size   | Description                        |
| ------------------ | ------------- | ------ | ---------------------------------- |
| `table_id`         | `uint64_t`    | 8      | Table unique identifier            |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_enter {
    uint64_t table_id;
};
```

#### Enter Res

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `SERVER_TO_CLIENT`                                              |

Fields:

| Field         | Type               | Size   | Description                                                     |
| ------------- | ------------------ | ------ | --------------------------------------------------------------- |
| `table_id`    | `uint64_t`         | 8      | Determines which table the response was for                     |
| `is_ok`       | `uint8_t`    | 1      | Is there any error?                                             |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_enter_res {
    pkrsrv_server_client_t* client;
    uint64_t table_id;
    unsigned char is_ok;
};
```

#### Leave

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `CLIENT_TO_SERVER`                                              |

Fields:

| Field         | Type               | Size   | Description                                                     |
| ------------- | ------------------ | ------ | --------------------------------------------------------------- |
| `table_id`    | `uint64_t`         | 8      | Determines which table the response was for                     |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_enter_res {
    uint64_t table_id;
};
```

#### Leave Res

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `SERVER_TO_CLIENT`                                              |

Fields:

| Field         | Type               | Size   | Description                                                     |
| ------------- | ------------------ | ------ | --------------------------------------------------------------- |
| `table_id`    | `uint64_t`         | 8      | Determines which table the response was for                     |
| `is_ok`       | `uint8_t`    | 1      | Is there any error?                                             |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_leave_res {
    uint64_t table_id;
    unsigned char is_ok;
};
```

#### Join

Joins into the game (takes a seat), must be sent while player is a watcher.

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `CLIENT_TO_SERVER`                                              |
| `has_response` | `true`                                                          |

Fields:

| Field              | Type          | Size   | Description                        |
| ------------------ | ------------- | ------ | ---------------------------------- |
| `table_id`         | `uint64_t`    | 8      | Table unique identifier            |
| `enterance_amount` | `uint64_t`    | 4      | Enterance amount to join with      |
| `position`         | `int32_t`     | 4      | Which seat player wanna sit        |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_join {
    uint64_t table_id;
    uint32_t enterance_amount;
    int32_t position;
};
```

##### Join Response

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `SERVER_TO_CLIENT`                                              |

Fields:

| Field         | Type               | Size   | Description                                                     |
| ------------- | ------------------ | ------ | --------------------------------------------------------------- |
| `table_id`    | `uint64_t`         | 8      | Determines which table the response was for                     |
| `is_ok`       | `uint8_t`    | 1      | Is there any error?                                             |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_join_res {
    uint64_t table_id;
    unsigned char is_ok;
};
```

#### Unjoin

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `CLIENT_TO_SERVER`                                              |
| `has_response` | `true`                                                          |

Fields:

| Field              | Type          | Size   | Description                        |
| ------------------ | ------------- | ------ | ---------------------------------- |
| `table_id`         | `uint64_t`    | 8      | Table unique identifier            |
| `enterance_amount` | `uint64_t`    | 4      | Enterance amount to join with      |
| `position`         | `int32_t`     | 4      | Which seat player wanna sit        |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_unjoin {
    uint64_t table_id;
};
```

##### Unjoin Response

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `SERVER_TO_CLIENT`                                              |

Fields:

| Field         | Type               | Size   | Description                                                     |
| ------------- | ------------------ | ------ | --------------------------------------------------------------- |
| `is_ok`       | `uint8_t`    | 1      | Is there any error?                                             |
| `table_id`    | `uint64_t`         | 8      | Determines which table the response was for                     |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_unjoin_res {
    unsigned char is_ok;
    uint64_t table_id;
};
```

#### Poker Info

```c
typedef struct pkrsrv_server_packet_frame_poker_info pkrsrv_server_packet_frame_poker_info_t;
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_poker_info {
    uint16_t name_length;
    uint16_t players_length;
    uint64_t table_id;
    uint16_t max_players;
    uint16_t action_timeout;
    uint64_t small_blind;
    uint64_t big_blind;
    uint64_t enterance_min;
    uint64_t enterance_max;
};
```

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `SERVER_TO_CLIENT`                                              |

Fields:

| Field            | Type               | Size   | Description                                                     |
| ---------------- | ------------------ | ------ | --------------------------------------------------------------- |
| `name_length`    | `uint16_t`         | 2      | Length of table name string (non-NULL-terminated)               |
| `players_length` | `uint16_t`         | 2      | Length of players array                                         |
| `table_id`       | `uint64_t`         | 8      | Table id that specifies which session is the game info for      |
| `max_player`     | `uint16_t`         | 2      | Number of max players for the table                             |
| `action_timeout` | `uint16_t`         | 2      | Action timeout (0 for unlimited)                                |
| `small_blind`    | `uint16_t`         | 2      | Small blind amount                                              |
| `big_blind`      | `uint16_t`         | 2      | Big blind amount                                                |
| `enterance_min`  | `uint64_t`         | 2      | Min amount of money player must have to join the table/session  |
| `enterance_max`  | `uint64_t`         | 2      | Max amount of money player must have to join the table/session  |

Next to receive:

| Field      | Type                                       | Size          | Description                             |
| ---------- | ------------------------------------------ | ------------- | --------------------------------------- |
| `name`     | `char*` to `pkrsrv_string_t` (ref-counted) | `name_length` | Game/table title (non-NULL-terminated)  |
| `pkrsrv_server_packet_frame_poker_info_player_t[]` | `char*` to `pkrsrv_string_t` (ref-counted) | `name_length` | Array of player frames |

```c
typedef struct pkrsrv_server_packet_frame_poker_info_player pkrsrv_server_packet_frame_poker_info_player_t;
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_poker_info_player {
    uint16_t name_length;
    uint32_t avatar_length;
    uint64_t id;
    uint8_t position;
    uint8_t is_playing;
    uint8_t is_dealt;
    uint8_t is_allin;
    uint8_t is_folded;
    uint8_t is_left;
    uint64_t balance;
};
```

| Field           | Type       | Size | Description                                     |
| --------------- | ---------- | ---- | ----------------------------------------------- |
| `name_length`   | `uint16_t` | `2`  | Public name length of the player                |
| `avatar_length` | `uint32_t` | `4`  | Avatar binary length of the player              |
| `id`            | `uint64_t` | `8`  | Account ID of the player                        |
| `position`      | `uint8_t`  | `1`  | Player's position                               |
| `is_playing`    | `uint8_t`  | `1`  | Is player playing or waiting for next hand?     |
| `is_dealt`      | `uint8_t`  | `1`  | Are player's hand cards dealt?                  |
| `is_allin`      | `uint8_t`  | `1`  | Had the player been done all-in?                |
| `is_folded`     | `uint8_t`  | `1`  | Is player folded?                               |
| `is_left`       | `uint8_t`  | `1`  | Is player left? (Will be removed in next start) |
| `balance`       | `uint32_t` | `4`  | Player's chips amaount                          |

Next to receive for each `pkrsrv_server_packet_frame_poker_info_player_t`:

| Vector      | Type                             | Size               |
| ----------- | -------------------------------- | ------------------ |
| `id_token`     | `unsigned char[id_token_length]`    | `id_token_length`     |
| `name`      | `unsigned char[name_length]`     | `name_length`      |
| `avatar`    | `unsigned char[avatar_length]`   | `avatar_length`    |

#### Poker State

The game state packet

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_poker_state {
    uint64_t table_id;
    uint16_t state;
    unsigned char is_playing;
    unsigned char cards[5];
    uint16_t players_length;
    unsigned char position;
    unsigned char playing_position;
    unsigned char is_dealt;
    unsigned char hand_cards[2];
    uint64_t balance;
    uint64_t bet;
    uint64_t current_raise;
    uint64_t current_bet;
    uint64_t pot_amount;
    uint64_t starting_after;
};
```

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `SERVER_TO_CLIENT`                                              |

Fields:

| Field              | Type               | Size | Description                                                                |
| ------------------ | ------------------ | ---- | -------------------------------------------------------------------------- |
| `table_id`         | `uint64_t`         | 8    | Determines which table                                                     |
| `state`            | `uint16_t`         | 2    | Game state identifier                                                      |
| `is_playing`       | `uint8_t`          | 1    | Is receiver player playing?                                                |
| `cards`            | `unsigned char[5]` | 5    | Cards array each byte has (4 bits card) + (4 bits kind)                    |
| `players_length`   | `uint16_t`         | 2    | Length of players array                                                    |
| `position`         | `uint8_t`          | 1    | Receiver player's position                                                 |
| `playing_position` | `uint8_t`          | 1    | Playing player's position                                                  |
| `is_dealt`         | `uint8_t`          | 1    | Are hand cards dealt?                                                      |
| `hand_cards`       | `unsigned char[2]` | 2    | Receiver player's hand cards. Each byte has (4 bits value) + (4 bits kind) |
| `balance`          | `uint64_t`         | 8    | Receiver player's current balance                                          |
| `bet`              | `uint64_t`         | 8    | Receiver player's current bet                                              |
| `current_raise`    | `uint64_t`         | 8    | Current raise-by amount                                                    |
| `current_bet`      | `uint64_t`         | 8    | Current bet amount                                                         |
| `pot_amount`       | `uint64_t`         | 8    | Total amount on pot                                                        |

Next to receive:

Next to receive:

| Field      | Type                                       | Size          | Description                             |
| ---------- | ------------------------------------------ | ------------- | --------------------------------------- |
| `name`     | `char*` to `pkrsrv_string_t` (ref-counted) | `name_length` | Game/table title (non-NULL-terminated)      |
| `pkrsrv_server_packet_frame_poker_info_player_t[]` | `char*` to `pkrsrv_string_t` (ref-counted) | `name_length` | Array of player frames |

```c
typedef struct pkrsrv_server_packet_frame_poker_state_player pkrsrv_server_packet_frame_poker_state_player_t;
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_poker_state_player {
    uint64_t id;
    uint16_t name_length;
    unsigned char position;
    unsigned char is_playing;
    unsigned char is_dealt;
    unsigned char is_allin;
    unsigned char is_folded;
    unsigned char is_left;
    uint64_t balance;
};
```

| Field         | Type       | Size | Description                                     |
| ------------- | ---------- | ---- | ----------------------------------------------- |
| `name_length` | `uint16_t` | `2`  | Public name length of the player                |
| `position`    | `uint8_t`  | `1`  | Player's position                               |
| `is_playing`  | `uint8_t`  | `1`  | Is player playing or waiting for next hand?     |
| `is_dealt`    | `uint8_t`  | `1`  | Are player's hand cards dealt?                  |
| `is_allin`    | `uint8_t`  | `1`  | Had the player been done all-in?                |
| `is_folded`   | `uint8_t`  | `1`  | Is player folded?                               |
| `is_left`     | `uint8_t`  | `1`  | Is player left? (Will be removed in next start) |
| `balance`     | `uint64_t` | `8`  | Player's chips amaount                          |

Next to receive for each `pkrsrv_server_packet_frame_poker_info_player_t` (Latest `poker_info.players_length` times):

| Vector      | Type                             | Size               |
| ----------- | -------------------------------- | ------------------ |
| `name`      | `unsigned char[name_length]`     | `name_length`      |

#### Poker End

The packet sent when game is ended

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_poker_end {
    uint64_t table_id;
    uint64_t winner_account_id;
    uint64_t earned_amount;
};
```

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `SERVER_TO_CLIENT`                                              |

Fields:

| Field               | Type                     | Size   | Description                                                     |
| ------------------- | ------------------------ | ------ | --------------------------------------------------------------- |
| `table_id`          | `uint64_t`               | 8      | Determines which joined session                                 |
| `winner_account_id` | `uint64_t`               | 8      | Winner account ID                                               |
| `earned_amount`     | `uint64_t`               | 8      | Pot amount that the winner earned                               |
| `scores_length`     | `uint8_t`                | 1      | Determines the length of score objects of multiple winners      |

Next to receive for each `pkrsrv_server_packet_frame_poker_end_score` (`scores_length` times):

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_poker_end_score {
    uint64_t account_id;
    uint64_t bet_diff;
};
```

| Field        | Type       | Size | Description                             |
| ------------ | ---------- | ---- | --------------------------------------- |
| `account_id` | `uint64_t` | 8    | Account ID of this score                |
| `bet_diff`   | `uint64_t` | 8    | Diff between final bet and player's bet |

#### Poker Restarted

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `SERVER_TO_CLIENT`                                              |

Fields:

| Field               | Type                     | Size   | Description                                                     |
| ------------------- | ------------------------ | ------ | --------------------------------------------------------------- |
| `table_id`          | `uint64_t`               | 8      | Determines which joined session                                 |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_poker_restarted {
    uint64_t table_id;
};
```

#### Poker Action

Poker action packet for doing actions from client to server.

> **Note**
> This packet is for only doing actions from client to server.  When a player does an action, an action reflection packet will be broadcasted to all clients: `pkrsrv_server_packet_frame_poker_action_reflection_t`/`pkrsrv_server_packet_poker_action_reflection_t`.

```c
typedef struct pkrsrv_server_packet_frame_poker_action pkrsrv_server_packet_frame_poker_action_t;
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_poker_action {
    uint64_t table_id;
    uint16_t action_kind;
    uint64_t amount;
};
```

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `CLIENT_TO_SERVER`                                              |

Fields:

| Field            | Type                                   | Size   | Description                                                     |
| ---------------- | -------------------------------------- | ------ | --------------------------------------------------------------- |
| `table_id`       | `uint64_t`                             | 8      | Table ID to identify which session                              |
| `action_kind`    | `uint16_t` of (`pkrsrv_poker_actions_action_kind_t`) | 2      | Determines hich poker action                                    |
| `amount`         | `uint64_t`                             | 8      | Amount value for bets/raises                                    |

#### Poker Action Reflection

Poker action event that gets broadcasted by server to all clients in the session when a poker action is done.

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_poker_action_reflection {
    uint64_t table_id;
    uint64_t account_id;
    uint16_t action_kind;
    uint64_t amount;
};
```

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `SERVER_TO_CLIENT`                                              |

Fields:

| Field            | Type                                   | Size   | Description                                                     |
| ---------------- | -------------------------------------- | ------ | --------------------------------------------------------------- |
| `table_id`       | `uint64_t`                             | 8      | Table ID to identify which session                              |
| `account_id`     | `uint64_t`                             | 8      | Account ID to identify which player                             |
| `action_kind`    | `uint16_t` of (`pkrsrv_poker_actions_action_kind_t`) | 2      | Determines hich poker action                                    |
| `amount`         | `uint64_t`                             | 8      | Amount value for bets/raises                                    |

#### Unjoined

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `SERVER_TO_CLIENT`                                              |

Fields:

| Field               | Type                     | Size   | Description                                                     |
| ------------------- | ------------------------ | ------ | --------------------------------------------------------------- |
| `table_id`          | `uint64_t`               | 8      | Determines which joined session                                 |
| `table_reason       | `uint32_t`               | 4      | Unjoining reason                                                |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_unjoined {
    uint64_t table_id;
    uint32_t reason;
};
```

#### Get Tables

Clients request list of tables with this packet and server sends `pkrsrv_server_packet_frame_tables_t` packet
and following `length * pkrsrv_server_packet_frame_table_t` packets to the client this after request.

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `CLIENT_TO_SERVER`                                              |

Fields:

| Field               | Type                     | Size   | Description                                                     |
| ------------------- | ------------------------ | ------ | --------------------------------------------------------------- |
| `offset`            | `uint16_t`               | 2      | Starting offset for table list                                  |
| `table_length       | `uint16_t`               | 2      | Number of tables to retrieve from the `offset`                  |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_get_tables {
    uint16_t offset;
    uint16_t length;
};
```

#### Tables

This packet is followed by `length * pkrsrv_server_packet_frame_table_t` packets that must get retrieved.

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `SERVER_TO_CLIENT`                                              |

Fields:

| Field               | Type       | Size | Description                                    |
| ------------------- | ---------- | ---- | ---------------------------------------------- |
| `offset`            | `uint16_t` | 2    | Starting offset for table list                 |
| `table_length       | `uint16_t` | 2    | Number of tables to retrieve from the `offset` |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_tables {
    uint16_t offset;
    uint16_t length;
};
```

Next to receive (`length`-times of `pkrsrv_server_packet_frame_table_t`...):

| Field               | Type        | Size  | Description                              |
| ------------------- | ----------- | ----- | ---------------------------------------- |
| `id`                | `uint64_t`  | 8     | Table ID                                 |
| `small_blind`       | `uint32_t`  | 4     | Small-blind amount                       |
| `big_blind`         | `uint32_t`  | 4     | Big-blind amount                         |
| `max_players`       | `uint16_t`  | 2     | Number of max players of the table       |
| `players_count`     | `uint16_t`  | 2     | Number of playing players                |
| `watchers_count`    | `uint16_t`  | 2     | Number of watchers                       |
| `name_length`       | `uint16_t`  | 2     | Table name length to retrieve name bytes |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_table {
    uint64_t id;
    uint32_t small_blind;
    uint32_t big_blind;
    uint16_t max_players;
    uint16_t players_count;
    uint16_t watchers_count;
    uint16_t name_length;
};
```

Next to receive:

| Field      | Type    | Size              | Description                              |
| ---------- | ------- | ----------------- | ---------------------------------------- |
| `name`     | `char*` | `name_length`     | Table name (non-NULL-terminated)         |

#### Get Sessions

This packet is for clients to request a list of currently playing game sessions.

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `CLIENT_TO_SERVER`                                              |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_get_sessions {
    uint16_t offset;
    uint16_t length;
};
```

#### Sessions

When sessions are requested by client, server sends this packet.

Attributes:

| Attribute      | Description                                                     |
| -------------- | --------------------------------------------------------------- |
| `direction`    | `SERVER_TO_CLIENT`                                              |

```c
struct __attribute__((__packed__))
pkrsrv_server_packet_frame_sessions {
    uint16_t offset;
    uint16_t length;
};
```

Next to receive:

```c
pkrsrv_server_packet_frame_poker_info_t[sessions.length]
```

#### JSON

Bidirectional JSON packets for generic usage.

JSONs are sent with a header that has `header.opcode = PKRSRV_SERVER_OPCODE_JSON` and `header.length`.

```c
(pkrsrv_server_packet_frame_header) {
    .opcode = PKRSRV_SERVER_OPCODE_JSON,
    .length = json_length
}
```

The following `JSON_LENGTH` bytes are supposed to be sent/received which is actually the serialized JSON string.

#### WebSocket Mode

The server switches to WebSocket mode if the client sends a WebSocket handshake request for that client.

##### WebSocket Handshake

The server checks if the client sends a HTTP GET request to understand if the client wants to switch to WebSocket mode.

The protocol's packet header (`pkrsrv_server_packet_frame_header_t`) occupies 8 bytes in total: 4 bytes for opcode and 4 bytes for length; to understand if the client wants to switch to WebSocket mode, the server interprets the first packet header as a vector and checks it if it starts with `"GET /"`.

Something like this:

```c
ssize_t result;
pkrsrv_server_packet_frame_header_t header;

RECEIVE_FIRST_FRAME:

READ_OR_CLOSE(client->ssl, &header, sizeof(pkrsrv_server_packet_frame_header_t));

char* header_str = (char *) (&header);

client->is_websocket = (header_str[0] == 'G') &&
                        (header_str[1] == 'E') &&
                        (header_str[2] == 'T') &&
                        (header_str[3] == ' ') &&
                        (header_str[4] == '/');
```

### WebSocket Packet Structure

```text
| ----- | ---------------------------------- | ---------------------------------- |
|  Byte |              Data                  |             Description            |
| ----- | ---------------------------------- | ---------------------------------- |
|                                Web Socket Header                                |
| ----- | ---------------------------------- | ---------------------------------- |
|   0   |              FIN                   | 1 bit                              |
|   0   |              RSV1                  | 1 bit                              |
|   0   |              RSV2                  | 1 bit                              |
|   0   |              RSV3                  | 1 bit                              |
|   0   |              OPCODE                | 4 bits                             |
|   1   |              MASK                  | 1 bit                              |
|   1   |              PAYLOAD LENGTH        | 7 bits, or 7+16 bits, or 7+64 bits |
|   2   |              PAYLOAD LENGTH        | (if payload length is 126)         |
|   3   |              PAYLOAD LENGTH        | (if payload length is 126)         |
|   2   |              PAYLOAD LENGTH        | (if payload length is 127)         |
|   3   |              PAYLOAD LENGTH        | (if payload length is 127)         |
|   4   |              MASKING KEY           | 4 bytes                            |
| ----- | ---------------------------------- | ---------------------------------- |
|                    Packet Header (Beginning of the actual packet)               |
| ----- | ---------------------------------- | ---------------------------------- |
|   5   |              OPCODE                | 4 bytes                            |
|   9   |              LENGTH                | 4 bytes                            |
| ----- | ---------------------------------- | ---------------------------------- |
|                               ... Packet Data ...                               |
| ----- | ---------------------------------- | ---------------------------------- |
```
