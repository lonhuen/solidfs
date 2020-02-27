#include "../test/fuse/test_fuse_helper.h"
#include "../test/fuse/test_fuse.h"

int test_truncate(int len)
{
    const char *data = testdata;
    int datalen = testdatalen;
    int res;

    start_test("truncate(%u)", (int) len);
    res = create_file(testfile, data, datalen);
    if (res == -1)
        return -1;

    res = truncate(testfile, len);
    if (res == -1) {
        PERROR("truncate");
        return -1;
    }
    res = check_size(testfile, len);
    if (res == -1)
        return -1;
    if (len > 0) {
        if (len <= datalen) {
            res = check_data(testfile, data, 0, len);
            if (res == -1)
                return -1;
        } else {
            res = check_data(testfile, data, 0, datalen);
            if (res == -1)
                return -1;
            res = check_data(testfile, zerodata, datalen,
                     len - datalen);
            if (res == -1)
                return -1;
        }
    }
    res = unlink(testfile);
    if (res == -1) {
        PERROR("unlink");
        return -1;
    }
    res = check_nonexist(testfile);
    if (res == -1)
        return -1;
    success();
    return 0;
}

