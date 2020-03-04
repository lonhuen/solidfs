#pragma once
#include <fstream>
#include "storage/storage.h"
#include "common.h"

namespace solid {
    class FileStorage: public Storage {
    private:
        std::fstream file;
        const BlockID capacity;

    public:
        FileStorage(uint64_t nr_blocks, const char* path="/dev/vdb");
        ~FileStorage();
        void read_block(BlockID id, uint8_t* dst);
        void write_block(BlockID id, const uint8_t* src);
    };
};