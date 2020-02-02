#pragma once
#include "storage/storage.h"
#include "common.h"

class MemoryStorage : public Storage {
    
    private:
        const bid_t NR_BLOCKS;
        uint8_t* data;

    public:
        MemoryStorage(bid_t nr_blocks);
        ~MemoryStorage();
        int read_block(bid_t id, uint8_t* dst);
        int write_block(bid_t id, const uint8_t* src);
#ifdef DEBUG
        typedef void (*DumpF)(const uint8_t*); 
        void dump_block(bid_t id, DumpF f=nullptr);
        void dump_range(bid_t sid, bid_t eid,DumpF f=nullptr);
#endif

};