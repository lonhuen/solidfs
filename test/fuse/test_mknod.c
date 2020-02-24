#include "../test/fuse/test_fuse_helper.h"
#include "../test/fuse/test_fuse.h"

#ifndef __FreeBSD__
int test_mknod(void)
{
    int err = 0;
    int res;

    start_test("mknod");
    unlink(testfile);
    res = mknod(testfile, 0644, 0);
    if (res == -1) {
        PERROR("mknod");
        return -1;
    }
    res = check_type(testfile, S_IFREG);
    if (res == -1)
        return -1;
    err += check_mode(testfile, 0644);
    err += check_nlink(testfile, 1);
    err += check_size(testfile, 0);
    res = unlink(testfile);
    if (res == -1) {
        PERROR("unlink");
        return -1;
    }
    res = check_nonexist(testfile);
    if (res == -1)
        return -1;
    if (err)
        return -1;

    success();
    return 0;
}
#endif
