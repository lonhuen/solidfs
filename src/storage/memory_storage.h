#pragma once
#include "storage/storage.h"

class MemoryStorage : Storage {
    const static int NR_BLOCKS = 2048;
    
    private:
        uint8_t buffer[NR_BLOCKS * BLOCK_SIZE];

    public:
        MemoryStorage() {};
        ~MemoryStorage() {};
        int read_block(BLOCK_ID id, uint8_t* dst);
        int write_block(BLOCK_ID id, const uint8_t* src);
};