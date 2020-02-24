#include "../test/fuse/test_fuse_helper.h"
#include "../test/fuse/test_fuse.h"

int test_unlink(void) {
    const char *data = testdata;
    int datalen = testdatalen;
    int res;

    start_test("unlink");

    res = create_file(testfile, data, datalen);
    if (res == -1) {
        PERROR("creat");
        return -1;
    }

    res = check_exist(testfile);
    if (res == -1) {
        ERROR("file not exist: %s\n", testfile);
        return -1;
    }

    res = unlink(testfile);
    if (res == -1) {
        PERROR("unlink");
        return -1;
    }

    res = check_nonexist(testfile);
    if (res == -1) {
        ERROR("file should not exist: %s\n", testfile);
        return -1;
    }

    success();
    return 0;
}
