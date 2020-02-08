#include "block/block_manager.h"
#include "block/super_block.h"
namespace solid {
    class FreeListBlockManager: public BlockManager {
    public:
        FreeListBlockManager(Storage* p_storage) : BlockManager(p_storage) {};

        const static BlockID nr_blocks_per_group = config::block_size / sizeof(BlockID);
            
        virtual void mkfs();
        virtual Block read_dblock(BlockID id);
        virtual void write_dblock(BlockID id, Block& src);
        virtual BlockID allocate_dblock();
        virtual void free_dblock(BlockID id);

    private:
        // since super block doesn't usually change its config, let's cache it.
        super_block sblock;
        bool initialized;
    };

};