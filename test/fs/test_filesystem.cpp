#include <iostream>
#include <algorithm>
#include <gtest/gtest.h>
#include "utils/log_utils.h"
#include "storage/memory_storage.h"
#include "block/freelist_blockmanager.h"
#include "fs/file_system.h"
#include "inode/inode.h"
#include "block/block.h"
#include "common.h"

namespace solid {
    class FileSystemTest : public testing::Test {
    public:
    static bool existException(std::function<void(void)> f) {
        bool flag = false;
        try {
            f();
        } catch (const std::exception& e) {
            flag = true;
        }
        return flag;
    };

    protected:
        static FileSystem* fs;

    };
    // this config for write test passed!:w
    //FileSystem* FileSystemTest::fs = new FileSystem(10 + 512 + 512 * 512 + 512 * 512 ,9);
    FileSystem* FileSystemTest::fs = new FileSystem(10 + 512 + 512 * 512,9);
    TEST_F(FileSystemTest,InitTest) {
        super_block sb;
        fs->storage->read_block(0,sb.data);
        //  EXPECT_EQ(sb.nr_block, 10+512+512*512 + 512*512);
        //  EXPECT_EQ(sb.nr_dblock, 10+512+512*512 +512 * 512- 10);
        EXPECT_EQ(sb.nr_block, 10+512+512*512  );
        EXPECT_EQ(sb.nr_dblock, 10+512+512*512 - 10);
        EXPECT_EQ(sb.s_dblock, 10);
        EXPECT_EQ(sb.s_iblock, 1);
        EXPECT_EQ(sb.nr_iblock, 9);

        fs->mkfs();
        INode inode = fs->im->read_inode(0);
        EXPECT_EQ(inode.size, 13);
        EXPECT_EQ(inode.block, 1);
        fs->im->write_inode(0,inode);
    }
    /* this won't work since we allocate 11 to root at the beginning
    TEST_F(FileSystemTest,ReadBlockIndexTest) {
        // first let's write to the root inode
        fs->mkfs();
        INode inode = fs->im->read_inode(0);
        EXPECT_EQ(inode.itype, INodeType::DIRECTORY);

        // let's write block entries
        for(int i=0;i<10;i++)
            inode.p_block[i] = i;
        
        // let's set the 1-level indirect lookup
        inode.p_block[10] = fs->bm->allocate_dblock();
        EXPECT_EQ(inode.p_block[10], 11);
        Block bl;
        
        for(int i=0;i<512;i++)
            bl.bl_entry[i] = i+10;
        fs->bm->write_dblock(11,bl);

        // let's set the 2-level indirect lookup
        inode.p_block[11] = fs->bm->allocate_dblock();
        EXPECT_EQ(inode.p_block[11], 12);

        for(int i=0;i<512;i++) {
            BlockID bid = fs->bm->allocate_dblock();
            bl.bl_entry[i] = bid;
            Block b;
        
            for(int j=0;j<512;j++) {
                b.bl_entry[j] = 10 + 512 + i * 512 + j;
            }
            fs->bm->write_dblock(bid,b);
        }
        fs->bm->write_dblock(inode.p_block[11],bl);

        // TODO(lonhh): this might need to verifed 
        // let's set the 3-level indirect lookup
        // but with only 1-1-1
        inode.p_block[12] = fs->bm->allocate_dblock();
        
        BlockID bid = fs->bm->allocate_dblock();
        bl.bl_entry[0] = bid;

        Block b2;
        BlockID bid2 = fs->bm->allocate_dblock();
        b2.bl_entry[0] = bid2;
        
        Block b3;
        for(int i=0;i<512;i++){
            b3.bl_entry[i] = 10 + 512 + 512 * 512 + i;
        }
        fs->bm->write_dblock(bid2,b3);
        fs->bm->write_dblock(bid,b2);
        fs->bm->write_dblock(inode.p_block[12],bl);

        fs->im->write_inode(0,inode);

        std::vector<BlockID> v = fs->read_dblock_index(inode,0,10+512+512*512+512);
        // this test costs much time
        // for(auto i=0;i<10+512+512*512+512;i++) {
        //     EXPECT_EQ(v[i],i);
        // }
        // 

        for(auto i=0;i<10+512+512*2;i++) {
            EXPECT_EQ(v[i],i);
        }
        
        v = fs->read_dblock_index(inode,1,10+512);
        for(auto i=1;i<10+512;i++) {
            EXPECT_EQ(v[i-1],i);
        }
        
        v = fs->read_dblock_index(inode,20,10+512);
        for(auto i=20;i<10+512;i++) {
            EXPECT_EQ(v[i-20],i);
        }
    }
    */
    TEST_F(FileSystemTest,ReadTest) {
        // let's write the first 20 blocks
        fs->mkfs();
        INode inode = fs->im->read_inode(0);
        EXPECT_EQ(inode.itype, INodeType::DIRECTORY);

        // let's write block entries
        for(int i=0;i<10;i++)
            inode.p_block[i] = fs->bm->allocate_dblock();
        
        // let's set the 1-level indirect lookup
        inode.p_block[10] = fs->bm->allocate_dblock();
        Block bl;
        
        for(int i=0;i<10;i++)
            bl.bl_entry[i] = fs->bm->allocate_dblock();
        fs->bm->write_dblock(inode.p_block[10],bl);

        inode.size = 20 * config::block_size;
        fs->im->write_inode(0,inode);

        std::vector<BlockID> v = fs->read_dblock_index(inode,0,20);
    for(auto i=0;i<20;i++) {
            Block tmp;
            // suppose we are writing in units of uint64_t
            for(uint32_t j=0;j<512;j++) {
                tmp.bl_entry[j] = i * 512 + j;
            }
            fs->bm->write_dblock(v[i],tmp);
    }
        uint8_t buffer[20 * config::block_size];

        // full range
        fs->read(0,buffer,20*config::block_size,0);
        for(auto i=0;i<20;i++) {
            for(auto j=0;j<512;j++) {
                auto ptr = ((BlockID*)buffer) + i * 512 + j;
                EXPECT_EQ(*ptr, i * 512 + j);
            }
        }
        
        // test offset assign to block
        fs->read(0,buffer,20*config::block_size,10 * config::block_size);
        for(auto i=0;i<10;i++) {
            for(auto j=0;j<512;j++) {
                auto ptr = ((BlockID*)buffer) + i * 512 + j;
                EXPECT_EQ(*ptr, (i+10) * 512 + j);
            }
        }
        // test offset inside a block
        fs->read(0,buffer,20*config::block_size,10 * config::block_size + config::block_size/2);
        for(auto j=0;j<256;j++) {
            auto ptr = ((BlockID*)buffer) + j;
            EXPECT_EQ(*ptr, 10*512 + j + 256);
        }
        for(auto i=1;i<10;i++) {
            for(auto j=0;j<512;j++) {
                auto ptr = ((BlockID*)buffer) + i * 512 + j - 256;
                EXPECT_EQ(*ptr, (i+10) * 512 + j);
            }
        }
    }
    TEST_F(FileSystemTest,NewDBlockTest) {
        // fs->mkfs();
        // auto nr = 10 + 512 + 512 * 7;
        // for(auto i=0;i<nr;i++) {
        //     auto ret = fs->bm->allocate_dblock();
        //     EXPECT_GT(ret,0);
        //     if(!ret)
        //         std::cout << i << std::endl;
        // }
        // let's write the first 20 blocks
        fs->mkfs();
        INode inode = fs->im->read_inode(0);
        EXPECT_EQ(inode.itype, INodeType::DIRECTORY);
        std::vector<BlockID> allocated_block_array;

        auto nr = 10 + 512 + 512 * 7;
        auto size = 0;
        for(auto i=0;i<nr;i++) {
            auto ret = 0;
            try {
                ret = fs->new_dblock(inode);
            } catch (const fs_exception& e) {
                std::cout << e.what() << std::endl;
            }
            allocated_block_array.push_back(ret);
            size++;
        }
        std::vector<BlockID> v = fs->read_dblock_index(inode,0,size+1);
        allocated_block_array.push_back(11);
        std::sort(allocated_block_array.begin(),allocated_block_array.end());
        std::sort(v.begin(),v.end());
        EXPECT_EQ(allocated_block_array.size(),v.size());
        for(auto i=0;i<allocated_block_array.size();i++) {
            EXPECT_EQ(allocated_block_array[i],v[i]);
        }
    }
    TEST_F(FileSystemTest,WriteTest) {
        fs->mkfs();
        uint8_t buffer[] = "hello world";
        uint8_t b[13];
        fs->write(0,buffer,13,0);
        fs->read(0,b,13,0);
        for(auto i=0;i<13;i++) {
            EXPECT_EQ(b[i],buffer[i]);
        }
        std::vector<uint64_t> test_len_array;

        test_len_array.push_back(1);
        test_len_array.push_back(9);
        test_len_array.push_back(30);
        test_len_array.push_back(10 + 512);
        test_len_array.push_back(30 + 512);
        //test_len_array.push_back(10 + 512 + 512 * 512);

        std::for_each(test_len_array.begin(),test_len_array.end(),[](auto p){
            auto len = p * config::block_size;
            uint8_t* src = new uint8_t[len];
            uint8_t* dst = new uint8_t[len];
            
            fs->write(0,src,len,0);
            fs->read(0,dst,len,0);

            for(auto i=0;i<len;i++) {
                EXPECT_EQ(src[i],dst[i]);
            }
            delete []src;
            delete []dst;
            INode inode = fs->im->read_inode(0);
        });
    }
    TEST_F(FileSystemTest,DirectoryTest) {
        // initialization and read
        fs->mkfs();
        INode inode = fs->im->read_inode(0);
        EXPECT_EQ(inode.size,13);
        EXPECT_EQ(inode.block,1);
        EXPECT_EQ(inode.p_block[0],11);
        uint8_t buffer[13];
        //dr.serialize(buffer,13);
        EXPECT_EQ(fs->read(0,buffer,13,0),13);
        Directory dr = fs->read_directory(0);
        std::for_each(dr.entry_m.begin(),dr.entry_m.end(),[&](auto p){
            EXPECT_TRUE(p.first == "." || p.first == "..");
            EXPECT_TRUE(p.second == 0);
        });

        // write and read
        dr.insert_entry("home",1);
        dr.insert_entry("bin",2);
        dr.insert_entry("dev",3);
        fs->write_directory(0,dr);
        dr = fs->read_directory(0);
        // std::for_each(dr.entry_m.begin(),dr.entry_m.end(),[&](auto p){
        //     std::cout << p.first << " " << p.second << std::endl;
        // });
    }

    TEST_F(FileSystemTest,NewINodeTest) {
        // initialization and read
        fs->mkfs();
        INode inode = fs->im->read_inode(0);

        fs->new_inode("home",inode);
        fs->new_inode("etc",inode);
        fs->new_inode("bin",inode);
        Directory dr = fs->read_directory(0);

        EXPECT_EQ(dr.contain_entry("home"),1);
        EXPECT_EQ(dr.contain_entry("bin"),1);
        EXPECT_EQ(dr.contain_entry("etc"),1);
        INodeID ret = dr.get_entry("home");
        EXPECT_EQ(ret,1);
        ret = dr.get_entry("bin");
        EXPECT_EQ(ret,1);
        ret = dr.get_entry("etc");
        EXPECT_EQ(ret,1);
    }
    /* this takes much time
    TEST_F(FileSystemTest,DeleteDBlockTest) {
        fs->mkfs();
        INode inode = fs->im->read_inode(0);
        EXPECT_EQ(inode.itype, INodeType::DIRECTORY);
        std::vector<BlockID> allocated_block_array;

        auto size = 0;
        auto i = 0;
        while(1) {
            if (!existException([&](){
                allocated_block_array.push_back(fs->bm->allocate_dblock());
            })) {
                size++;
                i++;
            }
            else {
                break;
            }
        }

        std::for_each(allocated_block_array.begin(),allocated_block_array.end(),[&](auto p){
            fs->bm->free_dblock(p);
        });

        while(1) {
            if(existException([&](){fs->new_dblock(inode);})) {
                break;
            }
        }

        EXPECT_TRUE(inode.block!=1);
        while(inode.block != 1) {
        fs->delete_dblock(inode);
        }
        EXPECT_EQ(inode.block,1);

        auto s = 0;
        while(1) {
            if(existException([&](){fs->bm->allocate_dblock();})) {
                break;
            }
            s++;
        }
        EXPECT_EQ(s,size);
        // re-allocate datablocks again
    }
    */
    TEST_F(FileSystemTest,UnlinkTest) {
        fs->mkfs();
        INode inode = fs->im->read_inode(0);
        EXPECT_EQ(inode.itype, INodeType::DIRECTORY);
        fs->unlink(0);
        EXPECT_EQ(fs->im->allocate_inode(),0);
        EXPECT_EQ(fs->bm->allocate_dblock(),11);
    }
    TEST_F(FileSystemTest,TruncateTest) {

        auto block_size = 4096;
        fs->mkfs();
        INode inode = fs->im->read_inode(0);
        EXPECT_EQ(inode.itype, INodeType::DIRECTORY);

        fs->truncate(0,block_size);
        inode = fs->im->read_inode(0);
        EXPECT_EQ(inode.block, 1);
        EXPECT_EQ(inode.size, block_size);

        std::vector<BlockID> allocated_block_array;

        auto size = 0;
        auto i = 0;
        while(1) {
            if(existException([&](){allocated_block_array.push_back(fs->bm->allocate_dblock());})) {
                break;
            }
            size++;
            i++;
        }

        std::for_each(allocated_block_array.begin(),allocated_block_array.end(),[&](auto p){
            fs->bm->free_dblock(p);
        });

        while(1) {
            // data block won't change file size!
            if(existException([&](){fs->new_dblock(inode);})) {
                break;
            }
        }
        fs->truncate(0,1);
        inode = fs->im->read_inode(0);
        EXPECT_EQ(inode.block,1);
        EXPECT_EQ(inode.size,1);

        auto s = 0;
        while(1) {
            if(existException([&](){fs->bm->allocate_dblock();})) {
                break;
            }
            s++;
        }
        EXPECT_EQ(s,size);
        // re-allocate datablocks again
    }

};
