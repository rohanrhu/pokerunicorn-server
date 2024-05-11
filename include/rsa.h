/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#pragma once

/**
 * \defgroup rsa Cryptography
 * \brief RSA encryption, decryption, signing and verification things.
 */

/**
 * \addtogroup rsa
 * \ingroup rsa
 * @{
 */

#include <stdbool.h>
#include <gmp.h>

#include "ref.h"
#include "sugar.h"
#include "string.h"
#include "stdint.h"

#define PKRSRV_RSA_PUB_EXPONENT 65537
#define PKRSRV_RSA_PRIME_BITS 2048
#define PKRSRV_RSA_PRIME_REPS 50
#define PKRSRV_RSA_BLOCK_SIZE 128

#define PKRSRV_MPZ_ENDIAN 1
#define PKRSRV_MPZ_ORDER 1

typedef uint8_t pkrsrv_rsa_random_seed_adds_t;
#define PKRSRV_RSA_RANDOM_SEED_ADDS_MASK 20 // 1 to 2 ^ ((sizeof(pkrsrv_rsa_random_seed_adds_t) * 8) - 1)

typedef struct pkrsrv_rsa_keypair pkrsrv_rsa_keypair_t;
typedef struct pkrsrv_rsa_key pkrsrv_rsa_key_t;
typedef struct pkrsrv_rsa_encrypted pkrsrv_rsa_encrypted_t;

/**
 * \implements pkrsrv_ref_counted
 * RSA key object
 */
struct pkrsrv_rsa_key {
    PKRSRV_REF_COUNTEDIFY();
    mpz_t e;
    mpz_t n;
};

/**
 * \implements pkrsrv_ref_counted
 */
struct pkrsrv_rsa_keypair {
    PKRSRV_REF_COUNTEDIFY();
    pkrsrv_rsa_key_t* pubkey;
    pkrsrv_rsa_key_t* privkey;
};

/**
 * \implements pkrsrv_ref_counted
 */
struct pkrsrv_rsa_encrypted {
    PKRSRV_REF_COUNTEDIFY();
    pkrsrv_string_t* data;
    pkrsrv_rsa_key_t* key;
};

uint32_t pkrsrv_rsa_hash_h32(char* str);
uint32_t pkrsrv_rsa_hash_h32__n(char* data, ssize_t length);

bool pkrsrv_rsa_random_seed(unsigned char* seed, size_t size);
void pkrsrv_rsa_random_prime(mpz_t num);

pkrsrv_rsa_key_t* pkrsrv_rsa_key_new();
void pkrsrv_rsa_key_free(pkrsrv_rsa_key_t* key);
pkrsrv_rsa_keypair_t* pkrsrv_rsa_keypair_new();
void pkrsrv_rsa_keypair_free(pkrsrv_rsa_keypair_t* keypair);

pkrsrv_rsa_encrypted_t* pkrsrv_rsa_encrypted_new();
void pkrsrv_rsa_encrypted_set_key(pkrsrv_rsa_encrypted_t* encrypted, pkrsrv_rsa_key_t* key);
void pkrsrv_rsa_encrypted_set_data(pkrsrv_rsa_encrypted_t* encrypted, pkrsrv_string_t* data);
void pkrsrv_rsa_encrypted_free(pkrsrv_rsa_encrypted_t* encrypted);
pkrsrv_rsa_encrypted_t* pkrsrv_rsa_encrypt(pkrsrv_string_t* data, pkrsrv_rsa_key_t* pubkey);
pkrsrv_string_t* pkrsrv_rsa_decrypt(pkrsrv_rsa_encrypted_t* encrypted, pkrsrv_rsa_key_t* privkey);

pkrsrv_string_t* pkrsrv_rsa_key_to_bin(pkrsrv_rsa_key_t* key);
pkrsrv_rsa_key_t* pkrsrv_rsa_key_from_bin(pkrsrv_string_t* hex);

pkrsrv_string_t* pkrsrv_rsa_key_to_hex(pkrsrv_rsa_key_t* key);
pkrsrv_rsa_key_t* pkrsrv_rsa_key_from_hex(pkrsrv_string_t* hex);

pkrsrv_string_t* pkrsrv_rsa_sign(pkrsrv_string_t* data, pkrsrv_rsa_key_t* privkey);
bool pkrsrv_rsa_verify(pkrsrv_string_t* data, pkrsrv_string_t* signature, pkrsrv_rsa_key_t* pubkey);
pkrsrv_string_t* pkrsrv_rsa_signature_to_hex(pkrsrv_string_t* signature_bin);
pkrsrv_string_t* pkrsrv_rsa_signature_from_hex(pkrsrv_string_t* signature_hex);

/**
 * @}
 */