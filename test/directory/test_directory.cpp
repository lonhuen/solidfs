
#include <iostream>
#include <gtest/gtest.h>
#include "utils/log_utils.h"
#include "directory/directory.h"

class DirectoryTest : public testing::Test {
public:
    static void SetUpTestCase() {
        LogUtils::log_level="3";
        LogUtils::init("test");
    }
};

TEST_F(DirectoryTest,InitTest) {
    Directory d(0,1);
    EXPECT_EQ(d.get_entry("."),0);
    EXPECT_EQ(d.get_entry(".."),1);
}

TEST_F(DirectoryTest,InsertTest) {
    Directory d(0,1);
    d.insert_entry("home",2);
    d.insert_entry("bin",3);
    d.insert_entry("etc",4);
    EXPECT_EQ(d.get_entry("home"),2);
    EXPECT_EQ(d.get_entry("bin"),3);
    EXPECT_EQ(d.get_entry("etc"),4);
}

TEST_F(DirectoryTest,RemoveTest) {
    Directory d(0,1);
    d.insert_entry("home",2);
    d.insert_entry("bin",3);
    d.insert_entry("etc",4);
    d.remove_entry("bin");
    EXPECT_EQ(d.get_entry("home"),2);
    EXPECT_EQ(d.get_entry("etc"),4);
}

TEST_F(DirectoryTest,SerializationTest) {
    Directory d(0,1);
    d.insert_entry("home",2);
    d.insert_entry("bin",3);
    d.insert_entry("etc",4);
    d.remove_entry("bin");

    uint8_t buffer[256];
    d.serialize(buffer,256);
    
    Directory t(0,1);
    t.deserialize(buffer,256);
    EXPECT_EQ(t.get_entry("home"),2);
    EXPECT_EQ(t.get_entry("etc"),4);
    EXPECT_EQ(t.get_entry("."),0);
    EXPECT_EQ(t.get_entry(".."),1);
    EXPECT_EQ(t.get_entry("bin"),0);
}