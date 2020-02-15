#include "../test/fuse/test_fuse_helper.h"
#include "../test/fuse/test_fuse.h"

int test_read_seek(void) {
    const char *data = testdata;
    int datalen = testdatalen;
    int res;
    int fd;

    // set up read buffer and read length
    char buf[2*sizeof(testdata)];
    int readlen;
    int offset;

    start_test("read & seek");

    // read from 0 to readlen <= datalen
    res = create_file(testfile, data, datalen);
    if (res == -1) {
        PERROR("creat");
        return -1;
    }

    // printf("access %s with return = %d\n", testfile, access(testfile, F_OK));
    fd = open(testfile, O_RDONLY);
    if (fd == -1) {
        PERROR("open");
        return -1;
    }

    readlen = datalen / 2;
    res = read(fd, buf, readlen);
    if (res == -1) {
        ERROR("case 1: read %d bytes from fd %d, buf = %s", res, fd, buf);
        close(fd);
        return -1;
    }
    if (res != readlen) {
        ERROR("incorrect readlen: %u instead of %u", res, readlen);
        close(fd);
        return -1;
    }
    if (strncmp(buf, data, readlen) != 0) {
        ERROR("incorrect read: %s instead of %s", buf, data);
        close(fd);
        return -1;
    }
    close(fd);

    // read from 0 to len > datalen
    res = create_file(testfile, data, datalen);
    if (res == -1) {
        PERROR("creat");
        return -1;
    }

    fd = open(testfile, O_RDONLY);
    if (fd == -1) {
        PERROR("open");
        return -1;
    }

    readlen = datalen + datalen / 2;
    res = read(fd, buf, readlen);
    if (res == -1) {
        ERROR("case 2: read %u bytes from fd %d", readlen, fd);
        close(fd);
        return -1;
    }  
    if (res != datalen) {
        ERROR("incorrect readlen: %u instead of %u", res, datalen);
        close(fd);
        return -1;
    }
    if (strncmp(buf, data, datalen) != 0) {
        ERROR("incorrect read: %s instead of %s", buf, data);
        close(fd); 
        return -1;
    }

    // read from offset <= datalen
    // with offset + readlen <= datalen
    res = create_file(testfile, data, datalen);
    if (res == -1) {
        PERROR("creat");
        return -1;
    }

    fd = open(testfile, O_RDONLY);
    if (fd == -1) {
        PERROR("open");
        return -1;
    }

    readlen = datalen / 2;
    offset = datalen - readlen;
    res = lseek(fd, offset, SEEK_SET);
    if (res == offset - 1) { 
        PERROR("lseek");
        close(fd);
        return -1;
    }
    if (res != offset) {
        ERROR("offset should have returned %u", offset);
        close(fd);
        return -1;
    }

    res = read(fd, buf, readlen);
    if (res == -1) {
        ERROR("case 3: read %u bytes from fd %d", readlen, fd);
        close(fd);
        return -1;
    }
    if (res != readlen) {
        ERROR("incorrect readlen: %u instead of %u", res, readlen);
        close(fd);
        return -1;
    }
    if (strncmp(buf, data + offset, readlen) != 0) {
        ERROR("incorrect read: %s instread of %s", buf, data + offset);
        close(fd);
        return -1;
    }
    close(fd);

    // read from offset <= datalen
    // with offset + readlen > datalen
    res = create_file(testfile, data, datalen);
    if (res == -1) {
        PERROR("creat");
        return -1;
    }

    fd = open(testfile, O_RDONLY);
    if (fd == -1) {
        PERROR("open");
        return -1;
    }
    
    readlen = datalen;
    offset = datalen / 2;
    
    res = lseek(fd, offset, SEEK_SET);
    if (res == offset - 1) {
        PERROR("lseek");
        close(fd);
        return -1;
    }
    if (res != offset) {
        ERROR("offset should have returned %u", offset);
        close(fd);
        return -1;
    }

    res = read(fd, buf, readlen);
    if (res == -1) {
        ERROR("case 4: read %u bytes from fd %d", readlen, fd);
        close(fd);
        return -1;
    }
    if (res != datalen - offset) {
        ERROR("incorrect readlen: %u instead of %u", res, datalen-offset);
        close(fd);
        return -1;
    }
    if (strncmp(buf, data + offset, datalen-offset) != 0) {
        ERROR("incorrect read: %s instread of %s", buf, data + offset);
        close(fd);
        return -1;
    }
    close(fd);

    // read from offset > datalen;
    offset = datalen + datalen / 2;
    res = lseek(fd, offset, SEEK_SET);
    if (res != -1) {
        ERROR("offset > filesize: %d instead of %d", res, offset-1);
        close(fd);
        return -1;
    }
 
    // all test passed
    success();
    return 0;       
}