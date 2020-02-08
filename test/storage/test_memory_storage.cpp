#include <iostream>
#include "storage/memory_storage.h"
#include "utils/log_utils.h"
#include <gtest/gtest.h>

namespace solid {
    GTEST_TEST(StorageTest,WriteRead) {
        BlockID nr_blocks = 10;
        MemoryStorage* ms = new MemoryStorage(nr_blocks);
        uint8_t buffer[config::block_size];
        uint8_t buffer2[config::block_size];

        for(int i=0;i<config::block_size;i++) {
            buffer[i] = i;
        }

        ms->write_block(0,buffer);
        ms->read_block(0,buffer2);

        for(int i=0;i<config::block_size;i++) {
            EXPECT_EQ(buffer[i],buffer2[i]) << "Data Differs at Block 0";
        }
    }

};