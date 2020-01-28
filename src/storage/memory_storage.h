#pragma once
#include "storage/storage.h"
#include "common.h"

class MemoryStorage : Storage {
    
    private:
        const BLOCK_ID NR_BLOCKS;
        uint8_t* data;

    public:
        MemoryStorage(BLOCK_ID nr_blocks);
        ~MemoryStorage();
        int read_block(BLOCK_ID id, uint8_t* dst);
        int write_block(BLOCK_ID id, const uint8_t* src);
#ifdef DEBUG
        typedef void (*DumpF)(const uint8_t*); 
        void dump_block(BLOCK_ID id, DumpF f=nullptr);
        void dump_range(BLOCK_ID sid, BLOCK_ID eid,DumpF f=nullptr);
#endif

};