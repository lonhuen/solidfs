#include "../test/fuse/test_fuse_helper.h"
#include "../test/fuse/test_fuse.h"

int test_readdir(void) {
    char parentdir[64];  // temp/testdir/
    char dirpath1[64];   // temp/testdir/testdir -- empty dir
    char dirpath2[64];   // temp/testdir/testdir2/ 
    char filepath1[64];  // temp/testdir/testfile -- content of parentdir
    char filepath2[64];  // temp/testdir/testdir2/testfile2
                         //  -- content of dirpath2
    const char *data1 = testdata;
    const char *data2 = testdata2;
    int datalen1 = testdatalen;
    int datalen2 = testdata2len;

    DIR *dir1;
    DIR *dir2;
    struct dirent *entry;
    int res;

    // construct path names
    strcpy(parentdir, testdir);
    
    strcpy(dirpath1, testdir);
    strcat(dirpath1, "/testdir");
    
    strcpy(dirpath2, testdir);
    strcat(dirpath2, "/testdir2");

    strcpy(filepath1, parentdir);
    strcat(filepath1, "/testfile");

    strcpy(filepath2, dirpath2);
    strcat(filepath2, "/testfile2");

    // create dir and file
    unlink(filepath1);
    unlink(filepath2);
    rmdir(dirpath1);
    rmdir(dirpath2);
    rmdir(parentdir);
    res = check_nonexist(parentdir);
    if (res == -1) {
        ERROR("%s should not exist", parentdir);
        return -1;
    }
    res = check_nonexist(dirpath1);
    if (res == -1) {
        ERROR("%s should not exist", dirpath1);
        return -1;
    }
    res = check_nonexist(dirpath2);
    if (res == -1) {
        ERROR("%s should not exist", dirpath2);
        return -1;
    }
    res = check_nonexist(filepath1);
    if (res == -1) {
        ERROR("%s should not exist", filepath1);
        return -1;
    }
    res = check_nonexist(filepath2);
    if (res != 0) {
        ERROR("%s should not exist", filepath2);
        return -1;
    }

    res = mkdir(parentdir, 775) + mkdir(dirpath1, 775) + mkdir(dirpath2, 775)
        + create_file(filepath1, data1, datalen1) 
        + create_file(filepath2, data2, datalen2);
    if (res != 0) {
        ERROR("create dir and file failed");
        return -1;
    }
    res = check_exist(parentdir) + check_exist(dirpath1) 
        + check_exist(dirpath2) + check_data(filepath1, data1, 0, datalen1);
        + check_data(filepath2, data2, 0, datalen2);
    if (res != 0) {
        ERROR("created dir/file content not match");
        return -1;
    }

    start_test("readdir");

    // read parentdir
    if ((dir1 = opendir(parentdir)) == NULL) {
        PERROR("opendir");
        return -1;
    } else {
        int count = 0;
        while ((entry = readdir(dir1)) != NULL) {
            count += 1; 
        }
        closedir(dir1);
        if ((dir2 = opendir(dirpath2)) == NULL) {
            PERROR("opendir");
            return -1;
        } else {
            while ((entry = readdir(dir2)) != NULL) {
                count += 1;
            }
            closedir(dir2);
        }
    
        // testfile, testdir, testdir2, ., .., testdir2/testfile2
        // testdir2/., testdir2/..
        if (count != 8) {
            ERROR("incorrect file number: %d instead of 8", count);
            return -1;
        }
    }   
    
    // clean folder
    unlink(filepath1);
    unlink(filepath2);
    rmdir(dirpath1);
    rmdir(dirpath2);
    rmdir(parentdir);
    res = check_nonexist(parentdir);
    if (res == -1) {
        ERROR("here %s should not exist", parentdir);
        return -1;
    }
    res = check_nonexist(dirpath1);
    if (res == -1) {
        ERROR("%s should not exist", dirpath1);
        return -1;
    }
    res = check_nonexist(dirpath2);
    if (res == -1) {
        ERROR("%s should not exist", dirpath2);
        return -1;
    }
    res = check_nonexist(filepath1);
    if (res == -1) {
        ERROR("%s should not exist", filepath1);
        return -1;
    }
    res = check_nonexist(filepath2);
    if (res != 0) {
        ERROR("%s should not exist", filepath2);
        return -1;
    }

    success();
    return 0;
}
