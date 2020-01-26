#pragma once
#include "storage/storage.h"

class MemoryStorage : Storage {
    const static BLOCK_ID NR_BLOCKS = 64;
    
    private:
        uint8_t buffer[NR_BLOCKS * BLOCK_SIZE];

    public:
        MemoryStorage() {};
        ~MemoryStorage() {};
        int read_block(BLOCK_ID id, uint8_t* dst);
        int write_block(BLOCK_ID id, const uint8_t* src);
#ifdef DEBUG
        typedef void (*DumpF)(const uint8_t*); 
        void dump_block(BLOCK_ID id, DumpF f=nullptr);
        void dump_range(BLOCK_ID sid, BLOCK_ID eid,DumpF f=nullptr);
#endif

};