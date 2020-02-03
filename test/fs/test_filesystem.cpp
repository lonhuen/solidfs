
#include <iostream>
#include <gtest/gtest.h>
#include "utils/log_utils.h"
#include "storage/memory_storage.h"
#include "block/freelist_blockmanager.h"
#include "fs/file_system.h"

class FileSystemTest : public testing::Test {
};

TEST_F(FileSystemTest,path2iid) {
    /*
    FileSystem fs;
    fs.path2iid("/home/lonhuen/////../github/");
    EXPECT_EQ(fs.path2iid("/"),0);
    */
    const std::string path = "/";
    std::string::size_type p1,p2;
    p2 = path.find('/');
    p1 = 0;
    while(std::string::npos != p2) {
        if((p1 != p2)) {
            std::cout << path.substr(p1,p2-p1) << std::endl;
        }
        p1 = p2 + 1;
        p2 = path.find('/',p1);
    }
}