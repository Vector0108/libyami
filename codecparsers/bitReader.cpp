/*
 * Copyright 2016 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include "bitReader.h"

#define LOAD8BYTESDATA_BE(x) \
    (((uint64_t)((const uint8_t*)(x))[0] << 56) | \
     ((uint64_t)((const uint8_t*)(x))[1] << 48) | \
     ((uint64_t)((const uint8_t*)(x))[2] << 40) | \
     ((uint64_t)((const uint8_t*)(x))[3] << 32) | \
     ((uint64_t)((const uint8_t*)(x))[4] << 24) | \
     ((uint64_t)((const uint8_t*)(x))[5] << 16) | \
     ((uint64_t)((const uint8_t*)(x))[6] <<  8) | \
     ((uint64_t)((const uint8_t*)(x))[7]))

namespace YamiParser {

const uint32_t BitReader::CACHEBYTES = sizeof(unsigned long int);

BitReader::BitReader(const uint8_t *pdata, uint32_t size)
    : m_stream(pdata)
    , m_size(size)
    , m_cache(0)
    , m_loadBytes(0)
    , m_bitsInCache(0)
{
    assert(pdata && size);
}

void BitReader::loadDataToCache(uint32_t nbytes)
{
    unsigned long int tmp = 0;
    const uint8_t *pStart = m_stream + m_loadBytes;

    if (nbytes == 8)
        tmp = LOAD8BYTESDATA_BE(pStart);
    else {
        for (uint32_t i = 0; i < nbytes; i++) {
            tmp <<= 8;
            tmp |= pStart[i];
        }
    }

    m_cache = tmp;
    m_loadBytes += nbytes;
    m_bitsInCache = nbytes << 3;
}

inline uint32_t BitReader::extractBitsFromCache(uint32_t nbits)
{
    if(!nbits)
        return 0;
    uint32_t tmp = 0;
    tmp = m_cache << ((CACHEBYTES << 3) - m_bitsInCache) >> ((CACHEBYTES << 3) - nbits);
    m_bitsInCache -= nbits;
    return tmp;
}

uint32_t BitReader::read(uint32_t nbits)
{
    assert(nbits <= CACHEBYTES << 3);

    /* Firstly loading data to m_cache, only need to read aligned bytes.
       So we can load 8 bytes which aligned with machine address in the next per time*/
    if (!m_loadBytes) {
        uint32_t alignedBytes = CACHEBYTES - ((uintptr_t)m_stream % CACHEBYTES);
        uint32_t toBeLoadSize = std::min(alignedBytes, m_size);
        loadDataToCache(toBeLoadSize);
    }

    uint32_t res = 0;

    if (nbits <= m_bitsInCache) {
        res = extractBitsFromCache(nbits);
    } else { /*not enough bits, need to save remaining bits
               in current cache and then reload more bits*/
        uint32_t toBeReadBits = nbits - m_bitsInCache;
        uint32_t tmp = extractBitsFromCache(m_bitsInCache);
        uint32_t remainingBytes = m_size - m_loadBytes;
        uint32_t toBeLoadSize = std::min(remainingBytes, CACHEBYTES);
        toBeReadBits = std::min(toBeReadBits, toBeLoadSize << 3);
        tmp <<= toBeReadBits;
        loadDataToCache(toBeLoadSize);
        res = tmp | extractBitsFromCache(toBeReadBits);
    }

    return res;
}

void BitReader::skipBits(uint32_t nbits)
{
    int32_t bits = nbits - m_bitsInCache;
    if (bits <= 0)
        m_bitsInCache -= nbits;
    else {
        uint32_t remainingBytes = m_size - m_loadBytes;
        uint32_t toBeLoadSize = std::min(remainingBytes, CACHEBYTES);
        assert(std::abs(bits) <= toBeLoadSize << 3);
        loadDataToCache(toBeLoadSize);
        m_bitsInCache -= bits;
    }
}

} /*namespace YamiParser*/
