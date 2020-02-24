#include "../test/fuse/test_fuse_helper.h"
#include "../test/fuse/test_fuse.h"

int do_test_open(int exist, int flags, const char *flags_str, int mode)
{
    char buf[4096];
    const char *data = testdata;
    int datalen = testdatalen;
    unsigned currlen = 0;
    int err = 0;
    int res;
    int fd;
    off_t off;

    start_test("open(%s, %s, 0%03o)", exist ? "+" : "-", flags_str, mode);
    unlink(testfile);
    if (exist) {
        res = create_file(testfile_r, testdata2, testdata2len);
        if (res == -1)
            return -1;

        currlen = testdata2len;
    }

    fd = open(testfile, flags, mode);
    if ((flags & O_CREAT) && (flags & O_EXCL) && exist) {
        if (fd != -1) {
            ERROR("open should have failed");
            close(fd);
            return -1;
        } else if (errno == EEXIST)
            goto succ;
    }
    if (!(flags & O_CREAT) && !exist) {
        if (fd != -1) {
            ERROR("open should have failed");
            close(fd);
            return -1;
        } else if (errno == ENOENT)
            goto succ;
    }
    if (fd == -1) {
        PERROR("open");
        return -1;
    }

    if (flags & O_TRUNC)
        currlen = 0;

    err += check_type(testfile, S_IFREG);
    if (exist)
        err += check_mode(testfile, 0644);
    else
        err += check_mode(testfile, mode);
    err += check_nlink(testfile, 1);
    err += check_size(testfile, currlen);
    if (exist && !(flags & O_TRUNC) && (mode & S_IRUSR))
        err += check_data(testfile, testdata2, 0, testdata2len);

    res = write(fd, data, datalen);
    if ((flags & O_ACCMODE) != O_RDONLY) {
        if (res == -1) {
            PERROR("write");
            err --;
        } else if (res != datalen) {
            ERROR("write is short: %u instead of %u", res, datalen);
            err --;
        } else {
            if (datalen > (int) currlen)
                currlen = datalen;

            err += check_size(testfile, currlen);

            if (mode & S_IRUSR) {
                err += check_data(testfile, data, 0, datalen);
                if (exist && !(flags & O_TRUNC) &&
                    testdata2len > datalen)
                    err += check_data(testfile,
                              testdata2 + datalen,
                              datalen,
                              testdata2len - datalen);
            }
        }
    } else {
        if (res != -1) {
            ERROR("write should have failed");
            err --;
        } else if (errno != EBADF) {
            PERROR("write");
            err --;
        }
    }
    off = lseek(fd, SEEK_SET, 0);
    if (off == (off_t) -1) {
        PERROR("lseek");
        err--;
    } else if (off != 0) {
        ERROR("offset should have returned 0");
        err --;
    }
    res = read(fd, buf, sizeof(buf));
    if ((flags & O_ACCMODE) != O_WRONLY) {
        if (res == -1) {
            PERROR("read");
            err--;
        } else {
            int readsize =
                currlen < sizeof(buf) ? currlen : sizeof(buf);
            if (res != readsize) {
                ERROR("read is short: %i instead of %u",
                      res, readsize);
                err--;
            } else {
                if ((flags & O_ACCMODE) != O_RDONLY) {
                    err += check_buffer(buf, data, datalen);
                    if (exist && !(flags & O_TRUNC) &&
                        testdata2len > datalen)
                        err += check_buffer(buf + datalen,
                                    testdata2 + datalen,
                                    testdata2len - datalen);
                } else if (exist)
                    err += check_buffer(buf, testdata2,
                                testdata2len);
            }
        }
    } else {
        if (res != -1) {
            ERROR("read should have failed");
            err --;
        } else if (errno != EBADF) {
            PERROR("read");
            err --;
        }
    }

    res = close(fd);
    if (res == -1) {
        PERROR("close");
        return -1;
    }
    res = unlink(testfile);
    if (res == -1) {
        PERROR("unlink");
        return -1;
    }
    res = check_nonexist(testfile);
    if (res == -1)
        return -1;
    res = check_nonexist(testfile_r);
    if (res == -1)
        return -1;
    if (err)
        return -1;

succ:
    success();
    return 0;
}
