
#include <iostream>
#include <gtest/gtest.h>
#include "utils/log_utils.h"
#include "directory/directory.h"

class DirectoryTest : public testing::Test {
public:
    static void SetUpTestCase() {
        LogUtils::log_level="0";
        LogUtils::init("test");
    }
};

TEST_F(DirectoryTest,InitTest) {
    Directory d(0,1);
    iid_t r;
    EXPECT_EQ(d.get_entry(".",&r),1);
    EXPECT_EQ(r,0);
    EXPECT_EQ(d.get_entry("..",&r),1);
    EXPECT_EQ(r,1);
}

TEST_F(DirectoryTest,InsertTest) {
    Directory d(0,1);
    d.insert_entry("home",2);
    d.insert_entry("bin",3);
    d.insert_entry("etc",4);
    
    iid_t r;
    EXPECT_EQ(d.get_entry("home",&r),1);
    EXPECT_EQ(r,2);
    EXPECT_EQ(d.get_entry("bin",&r),1);
    EXPECT_EQ(r,3);
    EXPECT_EQ(d.get_entry("etc",&r),1);
    EXPECT_EQ(r,4);
}

TEST_F(DirectoryTest,RemoveTest) {
    Directory d(0,1);
    d.insert_entry("home",2);
    EXPECT_EQ(d.insert_entry("bin",3),1);
    EXPECT_EQ(d.contain_entry("bin"),1);
    d.insert_entry("etc",4);
    EXPECT_EQ(d.remove_entry("bin"),1);
    iid_t r;
    EXPECT_EQ(d.get_entry("home",&r),1);
    EXPECT_EQ(r,2);
    EXPECT_EQ(d.get_entry("bin",&r),0);
    EXPECT_EQ(d.get_entry("etc",&r),1);
    EXPECT_EQ(r,4);
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
    iid_t r;
    EXPECT_EQ(t.get_entry("home",&r),1);
    EXPECT_EQ(r,2);
    EXPECT_EQ(t.get_entry("bin",&r),0);
    EXPECT_EQ(t.get_entry("etc",&r),1);
    EXPECT_EQ(r,4);
    EXPECT_EQ(t.get_entry(".",&r),1);
    EXPECT_EQ(r,0);
    EXPECT_EQ(t.get_entry("..",&r),1);
    EXPECT_EQ(r,1);
}