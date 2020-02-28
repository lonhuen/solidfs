#include "test_fuse.h"
#include "test_fuse_helper.h"

int main(int argc, char *argv[]) {
    const char *basepath;
    const char *realpath;
    int err = 0;
    int a;
    int is_root;

    umask(0);
    if (argc < 2 || argc > 4) {
        fprintf(stderr, "usage: %s testdir [:realdir] [[-]test#]\n", argv[0]);
        return 1;
    }
    basepath = argv[1];
    realpath = basepath;
    for (a = 2; a < argc; a++) {
        char *endptr;
        char *arg = argv[a];
        if (arg[0] == ':') {
            realpath = arg + 1;
        } else {
            if (arg[0] == '-') {
                arg++;
                skip_test = strtoul(arg, &endptr, 10);
            } else {
                select_test = strtoul(arg, &endptr, 10);
            }
            if (arg[0] == '\0' || *endptr != '\0') {
                fprintf(stderr, "invalid number: '%s'\n", arg);
                return 1;
            }
        }
    }
    assert(strlen(basepath) < 512);
    assert(strlen(realpath) < 512);
    if (basepath[0] != '/') {
        fprintf(stderr, "testdir must be an absolute path\n");
        return 1;
    }

    sprintf(testfile, "%s/testfile", basepath);
    sprintf(testfile2, "%s/testfile2", basepath);
    sprintf(testdir, "%s/testdir", basepath);
    sprintf(testdir2, "%s/testdir2", basepath);
    sprintf(subfile, "%s/subfile", testdir2);
    sprintf(testsock, "%s/testsock", basepath);

    sprintf(testfile_r, "%s/testfile", realpath);
    sprintf(testfile2_r, "%s/testfile2", realpath);
    sprintf(testdir_r, "%s/testdir", realpath);
    sprintf(testdir2_r, "%s/testdir2", realpath);
    sprintf(subfile_r, "%s/subfile", testdir2_r);

    is_root = (geteuid() == 0);
    
    err += test_read_seek();
    err += test_write();
    err += test_mkdir_add();
    err += test_readdir();
    err += test_rmdir_unlink();
    err += test_unlink();
    err += test_chmod();

    unlink(testfile);
    unlink(testfile2);
    rmdir(testdir);
    rmdir(testdir2);

    if (err) {
        fprintf(stderr, "%i tests failed\n", -err);
        return 1;
    }

    return 0;
}
