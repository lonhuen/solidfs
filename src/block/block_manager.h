#pragma once
#include "common.h"
#include "storage/storage.h"
#include "block/block.h"

namespace solid {
    /**
     * @brief manage all the data blocks
     * @param ptr_sblock: a pointer to the super_block
    */
    class BlockManager {
        protected:
            Storage* p_storage;

        public:
            BlockManager(Storage* p_storage) { this->p_storage = p_storage;}

            virtual void mkfs() = 0;
            virtual Block read_dblock(BlockID id) = 0;
            virtual void write_dblock(BlockID id, Block& src) = 0;
            virtual BlockID allocate_dblock() = 0;
            virtual void free_dblock(BlockID id) = 0;
    };
};