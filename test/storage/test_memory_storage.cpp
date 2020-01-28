#include <iostream>
#include "storage/memory_storage.h"
#include "utils/log_utils.h"
#include <gtest/gtest.h>


GTEST_TEST(StorageTest,WriteRead) {
    //LogUtils::log_level="3";
    LogUtils::init("test");

    BLOCK_ID nr_blocks = 10;
    MemoryStorage* ms = new MemoryStorage(nr_blocks);
    uint8_t buffer[BLOCK_SIZE];
    uint8_t buffer2[BLOCK_SIZE];

    for(int i=0;i<BLOCK_SIZE;i++) {
        buffer[i] = i;
    }

    ms->write_block(0,buffer);
    ms->read_block(0,buffer2);

//    ms->dump_block(0);
//    ms->dump_block(2049);
//    ms->dump_range(0,1);

    for(int i=0;i<BLOCK_SIZE;i++) {
        EXPECT_EQ(buffer[i],buffer2[i]) << "Data Differs at Block 0";
    }
}
