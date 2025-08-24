/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <openssl/sha.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#ifdef __APPLE__
#include <machine/endian.h>
#else
#include <endian.h>
#endif

#include "../include/websocket.h"
#include "../include/util.h"

int64_t pkrsrv_websocket_htonll(int64_t x) {
    if (1 == htonl(1)) {
        return x;
    }
    return ((int64_t) htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32);
}

int64_t pkrsrv_websocket_ntohll(int64_t x) {
    if (1 == ntohl(1)) {
        return x;
    }
    return ((int64_t) ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32);
}

static void pkrsrv_websocket_reset_fragmentation(pkrsrv_websocket_t* ws) {
    if (ws->buffer) {
        free(ws->buffer);
        ws->buffer = NULL;
    }
    ws->is_fragmented = false;
    ws->buffer_length = 0;
    ws->buffer_readed = 0;
}

char* pkrsrv_websocket_generate_ws_accept_key(char* websocket_key) {
    int combined_length = strlen(websocket_key) + strlen(PKRSRV_WEBSOCKET_GUID);
    char* combined = malloc(combined_length + 1);
    strcpy(combined, websocket_key);
    strcat(combined, PKRSRV_WEBSOCKET_GUID);

    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((unsigned char *) combined, combined_length, hash);

    char* accept_header = pkrsrv_util_base64_encode((unsigned char *) hash, SHA_DIGEST_LENGTH);

    free(combined);

    return accept_header;
}

extern ssize_t pkrsrv_websocket_read_http_header(pkrsrv_websocket_t* ws, SSL* ssl) {
    ssize_t result;
    ssize_t received = 0;

    int is_cr = 0;
    int is_lf = 0;

    char header_buff[HTTP_HEADER_BUFF_SIZE+1];
    strncpy(header_buff, PKRSRV_WEBSOCKET_EXPECTED_HTTP_HEADER, 8);
    
    char prop_buff[HTTP_PROP_BUFF_SIZE+1];
    char val_buff[HTTP_PROP_BUFF_SIZE+1];

    int header_buff_i = 8;
    int prop_buff_i = 0;
    int val_buff_i = 0;

    char* ws_key = NULL;

    char response_buf[700];

    enum PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE
    parser_state = PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_METHOD;

    char byte;

    RECEIVE_PACKET:

    if (received >= HTTP_HEADER_BUFF_SIZE) {
        goto FAIL;
    }

    result = ws->read(ssl, &byte, 1);
    if (!result) {
        goto FAIL;
    }

    received += result;

    if (parser_state == PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_METHOD) {
        if (!is_cr) {
            if (byte == '\r') {
                is_cr = 1;
            }
        } else {
            if (byte == '\n') {
                parser_state = PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_PROP;
                goto RECEIVE_PACKET;
            } else {
                goto FAIL;
            }
        }
        
        *(header_buff+(header_buff_i++)) = byte;

        if (!is_lf) {
            goto RECEIVE_PACKET;
        }

        is_lf = 0;

        const char* expected_header = PKRSRV_WEBSOCKET_EXPECTED_HTTP_HEADER;

        if (strncmp(header_buff, expected_header, strlen(PKRSRV_WEBSOCKET_EXPECTED_HTTP_HEADER) - 1) == 0) {
            goto RECEIVE_PACKET;
        } else {
            goto FAIL;
        }

        goto RECEIVE_PACKET;
    } else if (parser_state == PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_PROP) {
        if (byte == '\r') {
            parser_state = PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_END;
        } if (byte != ':') {
            *(prop_buff+(prop_buff_i++)) = byte;
        } else {
            parser_state = PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_SPACE;
            *(prop_buff+(prop_buff_i)) = '\0';
            prop_buff_i = 0;
        }
    } else if (parser_state == PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_SPACE) {
        if (byte != ' ') {
            goto FAIL;
        } else {
            parser_state = PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_VAL;
        }
    } else if (parser_state == PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_VAL) {
        if (byte == '\r') {
            parser_state = PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_CR;
            *(val_buff+(val_buff_i)) = '\0';
            val_buff_i = 0;
        } else {
            *(val_buff+(val_buff_i++)) = byte;
        }
    } else if (parser_state == PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_CR) {
        if (byte == '\n') {
            parser_state = PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_PROP;

            if (strcmp(prop_buff, "Sec-WebSocket-Key") == 0) {
                int len = strlen(val_buff);
                ws_key = malloc(len+1);
                ws_key[len] = '\0';
                strcpy(ws_key, val_buff);
            }
        } else {
            goto FAIL;
        }
    } else if (parser_state == PKRSRV_WEBSOCKET_HTTP_HEADER_PARSER_STATE_END) {
        if (byte == '\n') {
            if (!ws_key) {
                goto FAIL;
            }
            
            char* accept = pkrsrv_websocket_generate_ws_accept_key(ws_key);

            sprintf(
                response_buf,
                "HTTP/1.1 101 Switching Protocols\r\n"
                "Upgrade: websocket\r\n"
                "Connection: Upgrade\r\n"
                "Sec-WebSocket-Accept: %s\r\n"
                "Access-Control-Allow-Origin: *\r\n"
                "Compression: None\r\n\r\n",
                accept
            );

            free(accept);

            int written = ws->write(ssl, response_buf, strlen(response_buf));
            if (written <= 0) {
                goto FAIL;
            }
            
            goto SUCCESS;
        }
    }

    goto RECEIVE_PACKET;

    SUCCESS:

    if (ws_key) {
        free(ws_key);
    }

    return received;

    FAIL:

    if (ws_key) {
        free(ws_key);
    }

    return 0;
}

extern ssize_t pkrsrv_websocket_read_header(pkrsrv_websocket_t* ws, SSL* ssl) {
    ssize_t result;

    uint8_t header0_16[2];
    uint64_t plen;
    uint16_t plen16;
    uint64_t plen64;
    uint8_t opcode;
    uint8_t is_masked;
    bool fin;
    unsigned char mkey[4];

    READ_FRAME:

    result = ws->read(ssl, header0_16, 2);
    if (!result) {
        return 0;
    }

    opcode = header0_16[0] & 0b00001111;

    if (opcode == 8) {
        pkrsrv_websocket_reset_fragmentation(ws);
        return 0;
    }

    fin = header0_16[0] & 0b10000000;
    is_masked = header0_16[1] & 0b10000000;
    plen = header0_16[1] & 0b01111111;
    
    if (plen == 126) {
        result = ws->read(ssl, &plen16, 2);
        if (!result) {
            return 0;
        }

        plen = ntohs(plen16);
    } else if (plen == 127) {
        result = ws->read(ssl, &plen64, 8);
        if (!result) {
            return 0;
        }

        plen = pkrsrv_websocket_ntohll(plen64);
    }

    if (is_masked) {
        result = ws->read(ssl, mkey, 4);
        if (!result) {
            return 0;
        }
    }

    if ((opcode == 9) || (opcode == 10)) {
        if (!fin) {
            if (ws->buffer) {
                free(ws->buffer);
                ws->buffer = NULL;
                ws->is_fragmented = false;
                ws->buffer_length = 0;
                ws->buffer_readed = 0;
            }
            return 0;
        }
        
        unsigned char* payload = malloc(plen);
        if (!payload) {
            if (ws->buffer) {
                free(ws->buffer);
                ws->buffer = NULL;
                ws->is_fragmented = false;
                ws->buffer_length = 0;
                ws->buffer_readed = 0;
            }
            return 0;
        }

        ssize_t payload_result = ws->read(ssl, payload, plen);
        if (!payload_result) {
            free(payload);
            if (ws->buffer) {
                free(ws->buffer);
                ws->buffer = NULL;
                ws->is_fragmented = false;
                ws->buffer_length = 0;
                ws->buffer_readed = 0;
            }
            return 0;
        }

        if (is_masked) {
            for (int i=0; i < plen; i++) {
                payload[i] = payload[i] ^ mkey[i % 4];
            }
        }

        if (opcode == 9) {
            pkrsrv_websocket_packet_frame_len8_t pong_frame;
            pong_frame.fin_rsv_opcode = 0b10001010;
            pong_frame.mlen8 = plen;

            pthread_mutex_lock(ws->write_mutex);
            
            ssize_t pong_result = ws->write(ssl, &pong_frame, sizeof(pkrsrv_websocket_packet_frame_len8_t));
            if (!pong_result) {
                free(payload);
                if (ws->buffer) {
                    free(ws->buffer);
                    ws->buffer = NULL;
                    ws->is_fragmented = false;
                    ws->buffer_length = 0;
                    ws->buffer_readed = 0;
                }
                pthread_mutex_unlock(ws->write_mutex);
                return 0;
            }

            if (plen) {
                ssize_t pong_payload_result = ws->write(ssl, payload, plen);
                if (!pong_payload_result) {
                    free(payload);
                    if (ws->buffer) {
                        free(ws->buffer);
                        ws->buffer = NULL;
                        ws->is_fragmented = false;
                        ws->buffer_length = 0;
                        ws->buffer_readed = 0;
                    }
                    pthread_mutex_unlock(ws->write_mutex);
                    return 0;
                }
            }

            pthread_mutex_unlock(ws->write_mutex);
        }

        free(payload);

        goto READ_FRAME;
    }

    if (!fin && !ws->is_fragmented) {
        if (opcode < 1 || opcode > 2) {
            return 0;
        }
        
        ws->is_fragmented = true;
        ws->buffer = malloc(plen);
        if (!ws->buffer) {
            ws->is_fragmented = false;
            return 0;
        }
        ws->buffer_length = plen;
        ws->buffer_readed = 0;

        ssize_t payload_result = ws->read(ssl, ws->buffer, plen);
        if (!payload_result) {
            free(ws->buffer);
            ws->buffer = NULL;
            ws->is_fragmented = false;
            ws->buffer_length = 0;
            ws->buffer_readed = 0;
            return 0;
        }

        if (is_masked) {
            for (int i=0; i < plen; i++) {
                ws->buffer[i] = ws->buffer[i] ^ mkey[i % 4];
            }
        }

        goto READ_FRAME;
    } else if (ws->is_fragmented) {
        if (opcode != 0) {
            free(ws->buffer);
            ws->buffer = NULL;
            ws->is_fragmented = false;
            ws->buffer_length = 0;
            ws->buffer_readed = 0;
            return 0;
        }
        
        uint8_t* extended_buffer = realloc(ws->buffer, ws->buffer_length + plen);
        if (!extended_buffer) {
            free(ws->buffer);
            ws->buffer = NULL;
            ws->is_fragmented = false;
            ws->buffer_length = 0;
            ws->buffer_readed = 0;
            return 0;
        }
        ws->buffer = extended_buffer;

        ssize_t payload_result = ws->read(ssl, ws->buffer + ws->buffer_length, plen);
        if (!payload_result) {
            free(ws->buffer);
            ws->buffer = NULL;
            ws->is_fragmented = false;
            ws->buffer_length = 0;
            ws->buffer_readed = 0;
            return 0;
        }

        if (is_masked) {
            for (int i=0; i < plen; i++) {
                ws->buffer[(ws->buffer_length) + i] = ws->buffer[(ws->buffer_length) + i] ^ mkey[i % 4];
            }
        }
        
        ws->buffer_length += plen;

        if (!fin) {
            goto READ_FRAME;
        } else {
            return ws->buffer_length;
        }
    }

    ws->current_header.header0_16[0] = header0_16[0];
    ws->current_header.header0_16[1] = header0_16[1];
    ws->current_header.plen = plen;
    ws->current_header.plen16 = plen16;
    ws->current_header.plen64 = plen64;
    ws->current_header.opcode = opcode;
    ws->current_header.is_masked = is_masked;
    ws->current_header.fin = fin;
    ws->current_header.mkey[0] = mkey[0];
    ws->current_header.mkey[1] = mkey[1];
    ws->current_header.mkey[2] = mkey[2];
    ws->current_header.mkey[3] = mkey[3];
    ws->current_header.mask_i = 0;

    return plen;
}

extern ssize_t pkrsrv_websocket_read_payload(pkrsrv_websocket_t* ws, SSL* ssl, void* p_buffer, ssize_t length) {
    if (ws->is_fragmented) {
        unsigned char* buffer = (unsigned char*) p_buffer;
        
        ssize_t available = ws->buffer_length - ws->buffer_readed;
        if (length > available) {
            length = available;
        }
        
        if (length <= 0) {
            return 0;
        }
        
        memcpy(buffer, ws->buffer + ws->buffer_readed, length);
        ws->buffer_readed += length;

        if (ws->buffer_readed >= ws->buffer_length) {
            free(ws->buffer);
            ws->is_fragmented = false;
            ws->buffer = NULL;
            ws->buffer_length = 0;
            ws->buffer_readed = 0;
        }

        return length;
    }
    
    unsigned char* buffer = (unsigned char*) p_buffer;
    
    ssize_t result = ws->read(ssl, buffer, length);
    if (!result) {
        return 0;
    }

    if (ws->current_header.is_masked) {
        for (int i=0; i < length; i++) {
            buffer[i] = buffer[i] ^ ws->current_header.mkey[(ws->current_header.mask_i++) % 4];
        }
    }

    return result;
}

extern ssize_t pkrsrv_websocket_send_header(pkrsrv_websocket_t* ws, SSL* ssl, ssize_t length) {
    if (length < 126) {
        pkrsrv_websocket_packet_frame_len8_t message_frame;
        message_frame.fin_rsv_opcode = 0b10000010;
        message_frame.mlen8 = length;

        return ws->write(ssl, &message_frame, sizeof(pkrsrv_websocket_packet_frame_len8_t));
    }
    
    if (length < 65536) {
        pkrsrv_websocket_packet_frame_len16_t message_frame;
        message_frame.fin_rsv_opcode = 0b10000010;
        message_frame.mlen8 = 126;
        message_frame.len16 = htons(length);

        return ws->write(ssl, &message_frame, sizeof(pkrsrv_websocket_packet_frame_len16_t));
    }

    pkrsrv_websocket_packet_frame_len64_t message_frame;
    message_frame.fin_rsv_opcode = 0b10000010;
    message_frame.mlen8 = 127;
    message_frame.len64 = pkrsrv_websocket_htonll((int64_t) length);

    return ws->write(ssl, &message_frame, sizeof(pkrsrv_websocket_packet_frame_len64_t));
}

extern void pkrsrv_websocket_init(pkrsrv_websocket_t* ws) {
    ws->read = NULL;
    ws->write = NULL;
    ws->is_fragmented = false;
    ws->buffer = NULL;
    ws->buffer_length = 0;
    ws->buffer_readed = 0;
    ws->write_mutex = NULL;
}

extern void pkrsrv_websocket_cleanup(pkrsrv_websocket_t* ws) {
    pkrsrv_websocket_reset_fragmentation(ws);
}