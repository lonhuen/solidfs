#include "block/block_manager.h"
#include "block/super_block.h"

class FreeListBlockManager: BlockManager {
    public:
        FreeListBlockManager(Storage* p_storage) : BlockManager(p_storage) {};

        const static BLOCK_ID NR_BLOCKS_PER_GROUP = BLOCK_SIZE / sizeof(BLOCK_ID);
            
        virtual void mkfs();
        virtual int read_dblock(BLOCK_ID id, uint8_t* dst);
        virtual int write_dblock(BLOCK_ID id, const uint8_t* src);
        virtual int allocate_dblock();
        virtual int free_dblock(BLOCK_ID id);

    private:
        // since super block doesn't usually change its config, let's cache it.
        super_block sblock;
        bool initialized;
};