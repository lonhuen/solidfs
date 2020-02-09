#include <fstream>
#include <iostream>
#include <string.h>
#include <algorithm>
#include <gtest/gtest.h>
#include "utils/log_utils.h"

using namespace std;

class FSTest: public testing::Test {
    protected:
    ofstream of;
    ifstream inf;
    virtual void SetUp() {
        of.open("temp/a.txt");
        inf.open("temp/a.txt");
    }
};

TEST_F(FSTest,ReadWriteTest) {
    char buf1[] = "hello world!";
    char buf2[] = "            ";
    EXPECT_TRUE(of);
    EXPECT_TRUE(inf);
    
    of.write(buf1,strlen(buf1));
    of.close();

    inf.read(buf2,strlen(buf1));
    inf.close();

    for(auto i=0;i<strlen(buf1);i++)
        EXPECT_EQ(buf1[i],buf2[i]);
}