// taken from https://github.com/libfuse/libfuse/blob/master/test/test_syscalls.c
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
// #include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <utime.h>
#include <errno.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>

#ifndef ALLPERMS
# define ALLPERMS (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)/* 07777 */
#endif

#define MAX_ENTRIES 1024

extern char testfile[1024];
extern char testfile2[1024];
extern char testdir[1024];
extern char testdir2[1024];
extern char testsock[1024];
extern char subfile[1280];

extern char testfile_r[1024];
extern char testfile2_r[1024];
extern char testdir_r[1024];
extern char testdir2_r[1024];
extern char subfile_r[1280];

extern char testname[256];
extern char testdata[sizeof("abcdefghijklmnopqrstuvwxyz")];
extern char testdata2[sizeof("1234567890-=qwertyuiop[]\asdfghjkl;'zxcvbnm,./")];
extern const char *testdir_files[];
extern long seekdir_offsets[4];
extern char zerodata[4096];
extern int testdatalen;
extern int testdata2len;
extern unsigned int testnum;
extern unsigned int select_test;
extern unsigned int skip_test;

void test_perror(const char *func, const char *msg);

void test_error(const char *func, const char *msg, ...);

void __start_test(const char *fmt, ...);

void test_error(const char *func, const char *msg, ...);

int is_dot_or_dotdot(const char *name);

void success(void);

void __start_test(const char *fmt, ...);

#define start_test(msg, args...) { \
    if ((select_test && testnum != select_test) || \
        (testnum == skip_test)) { \
        testnum++; \
        return 0; \
    } \
    __start_test(msg, ##args);      \
}

#define PERROR(msg) test_perror(__FUNCTION__, msg)
#define ERROR(msg, args...) test_error(__FUNCTION__, msg, ##args)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

int check_size(const char *path, int len);

int check_type(const char *path, mode_t type);

int check_mode(const char *path, mode_t mode);

int check_nlink(const char *path, nlink_t nlink);

// helper function to check if file exists
int check_exist(const char *path);

int check_nonexist(const char *path);

int check_buffer(const char *buf, const char *data, unsigned len);

int check_data(const char *path, const char *data, int offset, unsigned len);

int check_dir_contents(const char *path, const char **contents);

int cleanup_dir(const char *path, const char **dir_files, int quiet);

int create_file(const char *path, const char *data, int len);