#include "../test/fuse/test_fuse_helper.h"
#include "../test/fuse/test_fuse.h"

int test_mkdir(void)
{
    int res;
    int err = 0;
    const char *dir_contents[] = {NULL};

    start_test("mkdir");
    rmdir(testdir);
    res = mkdir(testdir, 0755);
    if (res == -1) {
        PERROR("mkdir");
        return -1;
    }
    res = check_type(testdir, S_IFDIR);
    if (res == -1)
        return -1;
    err += check_mode(testdir, 0755);
    /* Some file systems (like btrfs) don't track link
       count for directories */
    //err += check_nlink(testdir, 2);
    err += check_dir_contents(testdir, dir_contents);
    res = rmdir(testdir);
    if (res == -1) {
        PERROR("rmdir");
        return -1;
    }
    res = check_nonexist(testdir);
    if (res == -1)
        return -1;
    if (err)
        return -1;

    success();
    return 0;
}
