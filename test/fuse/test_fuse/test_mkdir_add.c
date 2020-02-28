#include "../test/fuse/test_fuse_helper.h"
#include "../test/fuse/test_fuse.h"

int test_mkdir_add(void) {
    char dirpath[64];
    int res;

    // construct dir path
    strcpy(dirpath, "");
    strcat(dirpath, testdir2);
    strcat(dirpath, "/testdir");

    start_test("mkdir additional")
 
    // make 1 sub dir
    rmdir(testdir);
    res = mkdir(testdir, 0755);
    if (res == -1) {
        PERROR("mkdir");
        return -1;
    } 
    res = check_type(testdir, S_IFDIR);
    if (res == -1) {
        ERROR("%s not type directory", testdir);
        return -1;
    }
    res = check_mode(testdir, 0755);
    if (res == -1) {
        ERROR("mode != 0755");
        return -1;
    }

    // make nested dir, should return error
    rmdir(testdir);
    res = check_nonexist(testdir);
    if (res == -1) {
        ERROR("%s should not exist", testdir);
        return -1;
    }

    res = mkdir(dirpath, 0755);
    if (res != -1) {
        ERROR("should not create nested dir %s at once", dirpath);
        return -1;
    }
    res = check_nonexist(dirpath);
    if (res == -1) {
        ERROR("%s should not exist", dirpath);
        return -1;
    }

    // make nest dir from parent
    res = mkdir(testdir2, 0755);
    if (res == -1) {
        PERROR("mkdir");
        return -1;
    }
    res = mkdir(dirpath, 0755);
    if (res == -1) {
        ERROR("cannot create nested dir %s", dirpath);   
        return -1;
    }
    res = check_type(dirpath, S_IFDIR);
    if (res == -1) {
        ERROR("%s not type directory", dirpath);
        return -1;
    }
    res = check_mode(dirpath, 0755);
    if (res == -1) {
        ERROR("mode != 0755");
        return -1;
    }

    // clean up folder
    rmdir(dirpath);
    res = check_nonexist(dirpath);
    if (res == -1) {
        ERROR("%s should not exist", dirpath);
        return -1;
    }     
    rmdir(testdir2);
    res = check_nonexist(dirpath);
    if (res == -1) {
        ERROR("%s should not exist", dirpath);
        return -1;
    }

    // pass all tests
    success();
    return 0;
}
