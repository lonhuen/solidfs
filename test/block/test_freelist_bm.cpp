#include <iostream>
#include <gtest/gtest.h>
#include "utils/log_utils.h"
#include "utils/fs_exception.h"
#include "storage/memory_storage.h"
#include "block/freelist_blockmanager.h"
#include "block/super_block.h"
#include "block/block.h"

namespace solid {
    class BlockTest : public testing::Test {
    public:
    static bool existException(std::function<void(void)> f) {
        bool flag = false;
        try {
            f();
        } catch (const fs_exception& e) {
            flag = true;
        }
        return flag;
    };

    static void ignoreException(std::function<void(void)> f) {
        try {
            f();
        } catch (const fs_exception& e) {
            ;
        }
    };

    protected:
        static Storage* p_storage;
        static FreeListBlockManager* fbm;
        static BlockID nr_block;
        static BlockID s_iblock;
        static BlockID nr_iblock;
        static BlockID s_dblock;
        static BlockID nr_dblock;

    public:
        static void SetUpTestCase() {
            LogUtils::init("test");
            /* disk layout
            * 0 super_block
            * 1 - 9 inode
            * 10 -1299 data block
            */
            super_block sblock;
            sblock.nr_block = nr_block;
            sblock.s_iblock = s_iblock;
            sblock.nr_iblock = nr_iblock;
            sblock.s_dblock = s_dblock;
            sblock.nr_dblock= nr_block - nr_iblock -1;

            p_storage->write_block(0,(uint8_t*)(&sblock));

        }
    
        static void TearDownTestCase() {
            delete p_storage;
        }
    };

    BlockID BlockTest:: nr_block = 1300;
    BlockID BlockTest:: s_iblock = 1;
    BlockID BlockTest:: nr_iblock = 9;
    BlockID BlockTest:: s_dblock = 10;
    BlockID BlockTest:: nr_dblock = BlockTest::nr_block - BlockTest::nr_iblock - 1;
    Storage* BlockTest:: p_storage = (Storage*)new MemoryStorage(nr_block);
    FreeListBlockManager* BlockTest:: fbm = new FreeListBlockManager(p_storage);

    TEST_F(BlockTest,InitTest) {
        super_block sb;
        p_storage->read_block(0,sb.data);
        EXPECT_EQ(sb.nr_block , nr_block);
        EXPECT_EQ(sb.s_iblock, s_iblock);
        EXPECT_EQ(sb.nr_iblock, nr_iblock);
        EXPECT_EQ(sb.s_dblock, s_dblock);
        EXPECT_EQ(sb.nr_dblock, nr_dblock);
    }

    TEST_F(BlockTest,MkfsTest) {
        fbm->mkfs();
        Block db;
        BlockID delta = config::block_size/sizeof(BlockID);
        for(BlockID i=s_dblock;i<=nr_dblock+s_dblock;i+=config::block_size/sizeof(BlockID)){
            p_storage->read_block(i,db.data);
            if(i + delta < nr_block)
                EXPECT_EQ(db.fl_entry[0],i+delta) << "i delta nr_block " << i << " " << delta << " " << nr_block; 
            else
                EXPECT_EQ(db.fl_entry[0],0) << "i delta nr_block " << i << " " << delta << " " << nr_block; 
            for(int j=1;j<delta;j++){
                if(i + j < nr_block)
                    EXPECT_EQ(db.fl_entry[j],i+j);
                else
                    EXPECT_EQ(db.fl_entry[j],0);
            }
        }
    }
    TEST_F(BlockTest,AllocateDBlockTest) {
        Block db;
        BlockID delta = config::block_size/sizeof(BlockID);
        for(BlockID i=0;i<delta-1;i++) {
            EXPECT_EQ(i+s_dblock+1,fbm->allocate_dblock());
        }
        EXPECT_EQ(s_dblock,fbm->allocate_dblock());
        
        for(BlockID i=0;i<delta-1;i++) {
            EXPECT_EQ(i+s_dblock+1+delta,fbm->allocate_dblock());
        }
        EXPECT_EQ(s_dblock+delta,fbm->allocate_dblock());

        for(BlockID i=0;i<delta-1;i++) {
            if(i+s_dblock+1+delta+delta > nr_block) {
                bool flag = false;
                try {
                    fbm->allocate_dblock();
                } catch (const fs_exception& e) {
                    flag = true;
                }
                EXPECT_TRUE(flag);
            }
            else if(i+s_dblock+1+delta+delta == nr_block)
                EXPECT_EQ(delta*2 + s_dblock,fbm->allocate_dblock());
            else
                EXPECT_EQ(i+s_dblock+1+delta+delta,fbm->allocate_dblock());
        }
        if(s_dblock+3*delta >= nr_block) {
            bool flag = false;
            try {
                fbm->allocate_dblock();
            } catch (const fs_exception& e) {
                flag = true;
            }
            EXPECT_TRUE(flag);
        }
        super_block sb;
        p_storage->read_block(0,sb.data);
        EXPECT_EQ(0,sb.h_dblock);
    }
    // let's assume that no double free problems now
    TEST_F(BlockTest,FreeDBlockTest) {
        BlockID delta = config::block_size/sizeof(BlockID);

        // free the first 512 data blocks
        for(auto i=s_dblock;i<s_dblock+delta;i++) {
            ignoreException([&](){fbm->free_dblock(i);});
        }
        super_block sb;
        p_storage->read_block(0,sb.data);
        EXPECT_EQ(s_dblock,sb.h_dblock);
        Block b;
        p_storage->read_block(s_dblock,b.data);
        for(int i=1;i<delta;i++) {
            EXPECT_EQ(i+s_dblock,b.fl_entry[i]);
        }

        // free the second 512 data blocks
        for(auto i=s_dblock+delta;i<s_dblock + delta * 2;i++) {
            ignoreException([&](){fbm->free_dblock(i);});
        }
        p_storage->read_block(0,sb.data);
        EXPECT_EQ(s_dblock + delta,sb.h_dblock);
        p_storage->read_block(s_dblock+delta,b.data);
        for(int i=1;i<delta;i++) {
            if(delta+i+s_dblock < nr_block)
                EXPECT_EQ(delta+s_dblock+i,b.fl_entry[i]);
            else
                EXPECT_EQ(0,b.fl_entry[i]);
        }

        // free the third 512(not so many) data blocks
        for(auto i=s_dblock + delta*2;i<nr_block;i++) {
            ignoreException([&](){fbm->free_dblock(i);});
        }
        p_storage->read_block(0,sb.data);
        EXPECT_EQ(s_dblock + delta*2,sb.h_dblock);
        p_storage->read_block(s_dblock+delta*2,b.data);
        for(int i=1;i<delta;i++) {
            if(delta*2+i+s_dblock < nr_block)
                EXPECT_EQ(delta*2+s_dblock+i,b.fl_entry[i]);
            else
                EXPECT_EQ(0,b.fl_entry[i]);
        }
    }
    TEST_F(BlockTest,AllocateAfterFreeTest) {
        for(auto i=0;i<nr_dblock;i++)
            EXPECT_GE(fbm->allocate_dblock(),s_dblock);
        for(auto i=0;i<nr_dblock;i++) {
            EXPECT_TRUE(existException([&](){
                fbm->allocate_dblock();
            }));
        }
    }
};
