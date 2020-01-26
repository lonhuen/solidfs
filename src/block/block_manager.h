#pragma once
#include "common.h"
#include "storage/storage.h"

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
        virtual int read_dblock(BLOCK_ID id, uint8_t* dst) = 0;
        virtual int write_dblock(BLOCK_ID id, const uint8_t* dst) = 0;
        virtual int allocate_dblock() = 0;
        virtual int free_dblock(BLOCK_ID id) = 0;
};