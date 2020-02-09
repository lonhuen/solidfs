#include <iostream>
#include <gtest/gtest.h>
#include "utils/log_utils.h"
#include "utils/fs_exception.h"
#include "storage/memory_storage.h"
#include "inode/inode_manager.h"
#include "block/super_block.h"
#include "block/block.h"

namespace solid {
    class INodeTest : public testing::Test {

    protected:
        static Storage* p_storage;
        static INodeManager* im;
        static BlockID nr_block;
        static BlockID s_iblock;
        static BlockID nr_iblock;
        static BlockID s_dblock;
        static BlockID nr_dblock;

    public:
        static void SetUpTestCase() {
            LogUtils::log_level = "3";
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
            sblock.nr_dblock= nr_dblock;

            p_storage->write_block(0,(uint8_t*)(&sblock));

        }
    
        static void TearDownTestCase() {
            delete p_storage;
        }
    };

    BlockID INodeTest:: nr_block = 1300;
    BlockID INodeTest:: s_iblock = 1;
    BlockID INodeTest:: nr_iblock = 9;
    BlockID INodeTest:: s_dblock = 10;
    BlockID INodeTest:: nr_dblock = 1290;
    Storage* INodeTest:: p_storage = (Storage*)new MemoryStorage(nr_block);
    INodeManager* INodeTest:: im = new INodeManager(p_storage);

    TEST_F(INodeTest,InitTest) {
        super_block sb;
        p_storage->read_block(0,sb.data);
        EXPECT_EQ(sb.nr_block , nr_block);
        EXPECT_EQ(sb.s_iblock, s_iblock);
        EXPECT_EQ(sb.nr_iblock, nr_iblock);
        EXPECT_EQ(sb.s_dblock, s_dblock);
        EXPECT_EQ(sb.nr_dblock, nr_dblock);
    }

    TEST_F(INodeTest,MkfsTest) {
        im->mkfs();
        BlockID delta = config::block_size/sizeof(BlockID);
        for(BlockID i=s_iblock;i<=nr_iblock + s_iblock;i++){
            Block bl;
            p_storage->read_block(i,bl.data);
            for(auto j=0;j<INodeManager::nr_inode_per_block;j++){
                EXPECT_EQ(bl.inode[j].itype,INodeType::FREE);
            }
        }
    }

    TEST_F(INodeTest,AllocateINodeTest) {
        for(auto i=0;i<nr_iblock*INodeManager::nr_inode_per_block;i++){
            EXPECT_EQ(im->allocate_inode(),i);
            INode inode;
            inode.itype = INodeType::DIRECTORY;
            im->write_inode(i,inode);
        }
    }
    TEST_F(INodeTest,FreeINodeTest) {
        for(auto i=0;i<nr_iblock*INodeManager::nr_inode_per_block;i++){
            im->free_inode(i);
        }
    }

    TEST_F(INodeTest,AllocateAfterFreeTest) {
        for(auto i=0;i<nr_iblock*INodeManager::nr_inode_per_block;i++){
            EXPECT_EQ(im->allocate_inode(),i);
            INode inode;
            inode.itype = INodeType::DIRECTORY;
            im->write_inode(i,inode);
        }
    }
    TEST_F(INodeTest,DoubleFreeTest) {
        for(auto i=0;i<nr_iblock*INodeManager::nr_inode_per_block;i++){
            im->free_inode(i);
        }
        auto sum = 0;
        for(auto i=0;i<nr_iblock*INodeManager::nr_inode_per_block;i++){
            // * since we are not treating double free as a problem now, just ignore this
            //try {
                im->free_inode(i);
            //} catch(const std::exception& e){
            //    sum++;
            //};
        }
        //EXPECT_EQ(sum,nr_iblock*INodeManager::nr_inode_per_block);
        for(auto i=0;i<nr_iblock*INodeManager::nr_inode_per_block;i++){
            EXPECT_EQ(im->allocate_inode(),i);
            INode inode;
            inode.itype = INodeType::DIRECTORY;
            im->write_inode(i,inode);
        }
        im->free_inode(10);
        EXPECT_EQ(im->allocate_inode(),10);
    }
};