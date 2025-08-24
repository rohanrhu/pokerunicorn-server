/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

/**
 * \defgroup websocket Websocket
 * \brief WebSocket protocol implementation for the server module. This WebSocket library turns a TCP stream protocol into a WebSocket protocol.
 */

/**
 * \addtogroup websocket
 * \ingroup websocket
 * @{
 */

#include <stdint.h>
#include <stdbool.h>
#include <openssl/ssl.h>
#include <pthread.h>

#include "pkrsrv.h"
#include "ref.h"

#define PKRSRV_WEBSOCKET_EXPECTED_HTTP_HEADER "GET / HTTP/1.1"

#define pkrsrv_websocket_packet_frame_header \
    uint8_t fin_rsv_opcode; \
    uint8_t mlen8;
;

typedef struct pkrsrv_websocket_packet_frame_len8 pkrsrv_websocket_packet_frame_len8_t;
struct __attribute__((packed))
pkrsrv_websocket_packet_frame_len8 {
    pkrsrv_websocket_packet_frame_header
};

typedef struct pkrsrv_websocket_packet_frame_len16 pkrsrv_websocket_packet_frame_len16_t;
struct __attribute__((packed))
pkrsrv_websocket_packet_frame_len16 {
    pkrsrv_websocket_packet_frame_header
    uint16_t len16;
};

typedef struct pkrsrv_websocket_packet_frame_len64 pkrsrv_websocket_packet_frame_len64_t;
struct __attribute__((packed))
pkrsrv_websocket_packet_frame_len64 {
    pkrsrv_websocket_packet_frame_header
    // uint8_t len64[8];
    int64_t len64;
};

#define PKRSRV_WEBSOCKET_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

#define HTTP_HEADER_BUFF_SIZE 1000
#define HTTP_PROP_BUFF_SIZE 40
#define HTTP_VAL_BUFF_SIZE  50

enum PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE {
    PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_METHOD = 1,
    PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_PROP,
    PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_SPACE,
    PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_VAL,
    PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_CR,
    PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_END
};

enum PKRSRV_WEBSOCKET_RESPONSE {
    PKRSRV_WEBSOCKET_RESPONSE_INSTANCE_PORT = 1
};

typedef struct pkrsrv_websocket_header {
    uint8_t header0_16[2];
    uint8_t fin;
    uint64_t plen;
    uint16_t plen16;
    uint64_t plen64;
    uint8_t opcode;
    uint8_t is_masked;
    unsigned char mkey[4];
    int mask_i;
} pkrsrv_websocket_header_t;

typedef struct pkrsrv_websocket_payload {
    ssize_t length;
    unsigned char* data;
} pkrsrv_websocket_payload_t;

typedef struct pkrsrv_websocket {
    int (*read)(SSL* ssl, void* buffer, ssize_t length);
    int (*write)(SSL* ssl, void* buffer, ssize_t length);
    pkrsrv_websocket_header_t current_header;
    bool is_fragmented;
    char* buffer;
    int buffer_length;
    int buffer_readed;
    pthread_mutex_t* write_mutex;
} pkrsrv_websocket_t;

extern char* pkrsrv_websocket_generate_ws_accept_key(char* websocket_key);
extern void pkrsrv_websocket_init(pkrsrv_websocket_t* ws);
extern void pkrsrv_websocket_cleanup(pkrsrv_websocket_t* ws);
extern ssize_t pkrsrv_websocket_read_http_header(pkrsrv_websocket_t* ws, SSL* ssl);
extern ssize_t pkrsrv_websocket_read_header(pkrsrv_websocket_t* ws, SSL* ssl);
extern ssize_t pkrsrv_websocket_read_payload(pkrsrv_websocket_t* ws, SSL* ssl, void* buffer, ssize_t size);
extern ssize_t pkrsrv_websocket_send_header(pkrsrv_websocket_t* ws, SSL* ssl, ssize_t length);
extern void pkrsrv_websocket_fragmented_begin(pkrsrv_websocket_t* ws, SSL* ssl, size_t length);
extern ssize_t pkrsrv_websocket_fragmented_put(pkrsrv_websocket_t* ws, SSL* ssl, void* data, ssize_t length);

/**
 * @}
 */