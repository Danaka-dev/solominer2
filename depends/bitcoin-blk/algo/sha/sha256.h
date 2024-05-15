// Copyright (c) 2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CRYPTO_SHA256_H
#define BITCOIN_CRYPTO_SHA256_H

#include <stdint.h>
#include <stdlib.h>
#include <string>

#define SHA256_OUTPUT_SIZE  32

/** A hasher class for SHA-256. */
class CSHA256
{
private:
    uint32_t s[8];
    unsigned char buf[64];
    uint64_t bytes;

public:
    static const size_t OUTPUT_SIZE = SHA256_OUTPUT_SIZE;

    CSHA256();
    CSHA256& Write(const unsigned char* data, size_t len);
    void Finalize(unsigned char hash[OUTPUT_SIZE]);
    CSHA256& Reset();
};

/** Autodetect the best available SHA256 implementation.
 *  Returns the name of the implementation.
 */
std::string SHA256AutoDetect();

/** Compute SHA256's of input
 *
 * @param output:   pointer to a 32 byte output buffer
 * @param input:    pointer to input data
 * @param size:     size in byte of the input buffer
 */
void SHA256( unsigned char output[SHA256_OUTPUT_SIZE] ,const unsigned char *input ,size_t size );

/** Compute double-SHA256's of input
 *
 * @param output:   pointer to a 32 byte output buffer
 * @param input:    pointer to input data
 * @param size:     size in byte of the input buffer
 */
void SHA256D( unsigned char output[SHA256_OUTPUT_SIZE] ,const unsigned char *input ,size_t size );

/** Compute multiple double-SHA256's of 64-byte blobs.
 *  output:  pointer to a blocks*32 byte output buffer
 *  input:   pointer to a blocks*64 byte input buffer
 *  blocks:  the number of hashes to compute.
 */
void SHA256D64(unsigned char* output, const unsigned char* input, size_t blocks);

#endif // BITCOIN_CRYPTO_SHA256_H
