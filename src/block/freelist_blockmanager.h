#include "block/block_manager.h"

class FreeListBlockManager: BlockManager {
    public:
        FreeListBlockManager(Storage* p_storage) : BlockManager(p_storage) {};

    const static BLOCK_ID NR_BLOCKS_PER_GROUP = BLOCK_SIZE / sizeof(BLOCK_ID);
    
    void mkfs() = 0;
    int read_dblock(BLOCK_ID id, uint8_t* dst) = 0;
    int write_dblock(BLOCK_ID id, const uint8_t* dst) = 0;
    int allocate_dblock() = 0;
    int free_dblock(BLOCK_ID id) = 0;
};