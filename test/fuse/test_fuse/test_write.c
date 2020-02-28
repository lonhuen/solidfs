#include "../test/fuse/test_fuse_helper.h"
#include "../test/fuse/test_fuse.h"

int test_write() {
    const char *data = testdata;
    int datalen = testdatalen;
    int res;
    int fd;

    // set up read buffer and read length
    char buf[2*sizeof(testdata)];
    int writelen;
    int offset;

    start_test("write");

    // write new file
    res = create_file(testfile, data, datalen);
    if (res == -1) {
        PERROR("creat");
        return -1;
    }   
    res = check_data(testfile, data, 0, datalen);
    if (res == -1) {
        ERROR("read != write");
        return -1;
    }   

    // write existing file from start
    // write half of entire data
    writelen = datalen / 2;
    fd = open(testfile, O_RDWR);
    if (fd == -1) {
        PERROR("open");
        return -1;
    }

    res = lseek(fd, 0, SEEK_SET);
    if (res == -1) {
        PERROR("lseek");
        close(fd);
        return -1;
    }

    res = write(fd, data + datalen - writelen, writelen);
    if (res == -1) {
        PERROR("write");
        close(fd);
        return -1;
    }
    if (res != writelen) {
        ERROR("incorrect writelen: %u instead of %u", res, writelen);
    }
    res = check_data(testfile, data + datalen - writelen, 0, writelen);
    if (res == -1) {
        ERROR("read != write");
        close(fd);
        return -1;
    }

    // pass all tests
    success();
    return 0;
}

