#include "../test/fuse/test_fuse_helper.h"
#include "../test/fuse/test_fuse.h"

int test_rmdir_unlink(void) {
    char dirpath[64];
    char filepath[64];
    char filepath2[64];
    const char *data = testdata;
    int datalen = testdatalen;
    int res;

    // construct dirpath: testdir2/testdir
    strcpy(dirpath, "");
    strcat(dirpath, testdir2);
    strcat(dirpath, "/testdir");

    // construct filepath: testdir/testfile
    strcpy(filepath, "");
    strcat(filepath, testdir);
    strcat(filepath, "/testfile");

    // constrct filepath2: testdir2/testdir1/testfile
    strcpy(filepath2, "");
    strcat(filepath2, dirpath);
    strcat(filepath2, "/testfile");

    start_test("rmdir & unlink")   
    
    // remove empty dir
    unlink(filepath);
    rmdir(testdir);
    res = check_nonexist(testdir);
    if (res == -1) {
        ERROR("%s already exists", testdir);
        return -1;
    }
    res = mkdir(testdir, 0755);
    res = check_exist(testdir);
    if (res == -1) {
        ERROR("%s not created", testdir);
        return -1;
    }
    res = rmdir(testdir);
    if (res == -1) {
        PERROR("rmdir");
        return -1;
    }
    res = check_nonexist(testdir);
    if (res == -1) {
        ERROR("%s should be removed", testdir);
        return -1;
    }
      
    // remove non empty dir with file
    res = mkdir(testdir, 775);
    if (res == -1) {
        PERROR("mkdir");
        return -1;
    }
    res = create_file(filepath, data, datalen);
    if (res == -1) {
        PERROR("create_file");
        return -1;
    }
    res = check_exist(filepath);
    if (res == -1) {
        ERROR("%s not existed", filepath);
        return -1;
    }

    // 1. remove dir directly should fail
    res = rmdir(testdir);
    if (res != -1) {
        ERROR("should not remove non empty dir %s", testdir);
        return -1;
    }
    res = check_exist(testdir);
    if (res == -1) {
        ERROR("%s not existed", testdir);
        return -1;
    }
    res = check_exist(filepath);
    if (res == -1) {
        ERROR("%s not existed", filepath);
        return -1;
    }
    res = check_data(filepath, data, 0, datalen);
    if (res == -1) {
        ERROR("content changed for file %s", filepath);
        return -1;
    } 

    // 2. remove dir by remove file first 
    res = unlink(filepath);
    if (res == -1) {
        PERROR("unlink");
        return -1;
    }
    res = check_nonexist(filepath);
    if (res == -1) {
        ERROR("%s should be unlinked", filepath);
        return -1;
    }

    res = rmdir(testdir);
    if (res == -1) {
        PERROR("rmdir");
        return -1;
    }
    res = check_nonexist(testdir);
    if (res == -1) {
        ERROR("%s should be removed", testdir);
        return -1;
    }

    // remove non empty dir with sub dir    
    unlink(filepath2);
    rmdir(dirpath);
    rmdir(testdir2);
    res = check_nonexist(testdir2);
    if (res == -1) {
        ERROR("%s already exists", testdir2);
        return -1;
    }
    res = mkdir(testdir2, 775);
    if (res != 0) {
        ERROR("failed to create dir %s", testdir2);
        return -1;
    }
    res =mkdir(dirpath, 775);
    if (res != 0) {
        ERROR("failed to create dir %s", dirpath);
        return -1;
    }
    res = create_file(filepath2, data, datalen);
    if (res == -1) {
        ERROR("failed to create file %s", filepath2);
        return -1;
    }
    
    // 1. remove dir directly should fail
    res = rmdir(testdir2);
    if (res != -1) {
        ERROR("should not remove non empty dir %s", testdir2);
        return -1;
    }
    res = check_exist(testdir2) 
        + check_exist(dirpath) 
        + check_exist(filepath2);
    if (res != 0) {
        ERROR("file not exist");
        return -1;
    } 
    
    // 2. remove file, rmdir testdir2 should fail
    res = unlink(filepath2);
    if (res == -1) {
        PERROR("unlink");
        return -1;
    }
    res = check_nonexist(filepath2);
    if (res == -1) {
        ERROR("file should be unlinked %s", filepath2);
        return -1;
    }

    res = rmdir(testdir2);
    if (res != -1) {
        ERROR("should not remove non empty dir %s", testdir2);
        return -1;
    }
    res = check_exist(testdir2) + check_exist(dirpath);
    if (res != 0) {
        ERROR("file not existed");
        return -1;
    }

    // 3. remove sub dir
    res = rmdir(dirpath);
    if (res == -1) {
        PERROR("rmdir");
        return -1;
    }
    res = check_nonexist(dirpath);
    if (res == -1) {
        ERROR("should be removed: %s", dirpath);
        return -1;
    }

    // remove parent dir
    res = check_exist(testdir2);
    if (res == -1) {
        ERROR("file not exist: %s", testdir2);
        return -1;
    }
    res = rmdir(testdir2);
    if (res == -1) {
        PERROR("rmdir");
        return -1;
    }
    res = check_nonexist(testdir2);
    if (res == -1) {
        ERROR("should be removed: %s", testdir2);
        return -1;
    }
    
    // pass all tests
    success();
    return 0;
}
