#pragma once
#include "storage/storage.h"
#include "common.h"

namespace solid {
    class MemoryStorage : public Storage {
    private:
        const BlockID capacity;
        uint8_t* data;

    public:
        MemoryStorage(BlockID nr_blocks);
        ~MemoryStorage();
        void read_block(BlockID id, uint8_t* dst);
        void write_block(BlockID id, const uint8_t* src);
    };
};