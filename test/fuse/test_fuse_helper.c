// taken from https://github.com/libfuse/libfuse/blob/master/test/test_syscalls.c
#include "../test/fuse/test_fuse_helper.h"
#include "../test/fuse/test_fuse.h"

char testfile[1024];
char testfile2[1024];
char testdir[1024];
char testdir2[1024];
char testsock[1024];
char subfile[1280];

char testfile_r[1024];
char testfile2_r[1024];
char testdir_r[1024];
char testdir2_r[1024];
char subfile_r[1280];

char testname[256];
char testdata[] = "abcdefghijklmnopqrstuvwxyz";
char testdata2[] = "1234567890-=qwertyuiop[]\asdfghjkl;'zxcvbnm,./";
const char *testdir_files[] = { "f1", "f2", NULL};
long seekdir_offsets[4];
char zerodata[4096];
int testdatalen = sizeof(testdata) - 1;
int testdata2len = sizeof(testdata2) - 1;
unsigned int testnum = 1;
unsigned int select_test = 0;
unsigned int skip_test = 0;

void test_perror(const char *func, const char *msg)
{
    fprintf(stderr, "%s %s() - %s: %s\n", testname, func, msg,
        strerror(errno));
}

void test_error(const char *func, const char *msg, ...)
    __attribute__ ((format (printf, 2, 3)));

void __start_test(const char *fmt, ...)
    __attribute__ ((format (printf, 1, 2)));

void test_error(const char *func, const char *msg, ...)
{
    va_list ap;
    fprintf(stderr, "%s %s() - ", testname, func);
    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}

int is_dot_or_dotdot(const char *name) {
    return name[0] == '.' &&
           (name[1] == '\0' || (name[1] == '.' && name[2] == '\0'));
}

void success(void)
{
    fprintf(stderr, "%s OK\n", testname);
}

void __start_test(const char *fmt, ...)
{
    unsigned int n;
    va_list ap;
    n = sprintf(testname, "%3i [", testnum++);
    va_start(ap, fmt);
    n += vsprintf(testname + n, fmt, ap);
    va_end(ap);
    sprintf(testname + n, "]");
}

int check_size(const char *path, int len)
{
    struct stat stbuf;
    int res = stat(path, &stbuf);
    if (res == -1) {
        PERROR("stat");
        return -1;
    }
    if (stbuf.st_size != len) {
        ERROR("length %u instead of %u", (int) stbuf.st_size,
              (int) len);
        return -1;
    }
    return 0;
}

int check_type(const char *path, mode_t type)
{
    struct stat stbuf;
    int res = lstat(path, &stbuf);
    if (res == -1) {
        PERROR("lstat");
        return -1;
    }
    if ((stbuf.st_mode & S_IFMT) != type) {
        ERROR("type 0%o instead of 0%o", stbuf.st_mode & S_IFMT, type);
        return -1;
    }
    return 0;
}

int check_mode(const char *path, mode_t mode)
{
    struct stat stbuf;
    int res = lstat(path, &stbuf);
    if (res == -1) {
        PERROR("lstat");
        return -1;
    }
    if ((stbuf.st_mode & ALLPERMS) != mode) {
        ERROR("mode 0%o instead of 0%o", stbuf.st_mode & ALLPERMS,
              mode);
        return -1;
    }
    return 0;
}

int check_nlink(const char *path, nlink_t nlink)
{
    struct stat stbuf;
    int res = lstat(path, &stbuf);
    if (res == -1) {
        PERROR("lstat");
        return -1;
    }
    if (stbuf.st_nlink != nlink) {
        ERROR("nlink %li instead of %li", (long) stbuf.st_nlink,
              (long) nlink);
        return -1;
    }
    return 0;
}

// helper function to check if file exists
int check_exist(const char *path) {
    struct stat stbuf;
    int res = lstat(path, &stbuf);
    if (res == 0) {  // file exists
        return 0;
    } else {
        ERROR("file not exist");
        return -1;
    }
}

int check_nonexist(const char *path)
{
    struct stat stbuf;
    int res = lstat(path, &stbuf);
    if (res == 0) {
        ERROR("file should not exist");
        return -1;
    }
    if (errno != ENOENT) {
        ERROR("file should not exist: %s", strerror(errno));
        return -1;
    }
    return 0;
}

int check_buffer(const char *buf, const char *data, unsigned len)
{
    if (memcmp(buf, data, len) != 0) {
        ERROR("data mismatch");
        return -1;
    }
    return 0;
}

int check_data(const char *path, const char *data, int offset,
              unsigned len)
{
    char buf[4096];
    int res;
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        PERROR("open");
        return -1;
    }
    if (lseek(fd, offset, SEEK_SET) == (off_t) -1) {
        PERROR("lseek");
        close(fd);
        return -1;
    }
    while (len) {
        int rdlen = len < sizeof(buf) ? len : sizeof(buf);
        res = read(fd, buf, rdlen);
        if (res == -1) {
            PERROR("read");
            close(fd);
            return -1;
        }
        if (res != rdlen) {
            ERROR("short read: %u instead of %u", res, rdlen);
            close(fd);
            return -1;
        }
        if (check_buffer(buf, data, rdlen) != 0) {
            close(fd);
            return -1;
        }
        data += rdlen;
        len -= rdlen;
    }
    res = close(fd);
    if (res == -1) {
        PERROR("close");
        return -1;
    }
    return 0;
}

int check_dir_contents(const char *path, const char **contents)
{
    int i;
    int res;
    int err = 0;
    int found[MAX_ENTRIES];
    const char *cont[MAX_ENTRIES];
    DIR *dp;

    for (i = 0; contents[i]; i++) {
        assert(i < MAX_ENTRIES - 3);
        found[i] = 0;
        cont[i] = contents[i];
    }
    cont[i] = NULL;

    dp = opendir(path);
    if (dp == NULL) {
        PERROR("opendir");
        return -1;
    }
    memset(found, 0, sizeof(found));
    while(1) {
        struct dirent *de;
        errno = 0;
        de = readdir(dp);
        if (de == NULL) {
            if (errno) {
                PERROR("readdir");
                closedir(dp);
                return -1;
            }
            break;
        }
        if (is_dot_or_dotdot(de->d_name))
            continue;
        for (i = 0; cont[i] != NULL; i++) {
            assert(i < MAX_ENTRIES);
            if (strcmp(cont[i], de->d_name) == 0) {
                if (found[i]) {
                    ERROR("duplicate entry <%s>",
                          de->d_name);
                    err--;
                } else
                    found[i] = 1;
                break;
            }
        }
        if (!cont[i]) {
            ERROR("unexpected entry <%s>", de->d_name);
            err --;
        }
    }
    for (i = 0; cont[i] != NULL; i++) {
        if (!found[i]) {
            ERROR("missing entry <%s>", cont[i]);
            err--;
        }
    }
    res = closedir(dp);
    if (res == -1) {
        PERROR("closedir");
        return -1;
    }
    if (err)
        return -1;

    return 0;
}

int create_file(const char *path, const char *data, int len)
{
    int res;
    int fd;

    unlink(path);
    fd = creat(path, 0644);
    if (fd == -1) {
        PERROR("creat");
        return -1;
    }
    if (len) {
        res = write(fd, data, len);
        if (res == -1) {
            PERROR("write");
            close(fd);
            return -1;
        }
        if (res != len) {
            ERROR("write is short: %u instead of %u", res, len);
            close(fd);
            return -1;
        }
    }
    res = close(fd);
    if (res == -1) {
        PERROR("close");
        return -1;
    }
    res = check_type(path, S_IFREG);
    if (res == -1)
        return -1;
    res = check_mode(path, 0644);
    if (res == -1)
        return -1;
    res = check_nlink(path, 1);
    if (res == -1)
        return -1;
    res = check_size(path, len);
    if (res == -1)
        return -1;

    if (len) {
        res = check_data(path, data, 0, len);
        if (res == -1)
            return -1;
    }

    return 0;
}

int cleanup_dir(const char *path, const char **dir_files, int quiet)
{
    int i;
    int err = 0;

    for (i = 0; dir_files[i]; i++) {
        int res;
        char fpath[1280];
        sprintf(fpath, "%s/%s", path, dir_files[i]);
        res = unlink(fpath);
        if (res == -1 && !quiet) {
            PERROR("unlink");
            err --;
        }
    }
    if (err)
        return -1;

    return 0;
}
