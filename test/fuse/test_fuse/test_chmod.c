#include "../test/fuse/test_fuse_helper.h"
#include "../test/fuse/test_fuse.h"

int test_chmod_file(void);
int test_chmod_empty_dir(void);
int test_chmod_file_dir(void);
int test_chmod_nest_dir(void);

int test_chmod(void) {
    test_chmod_file();
    test_chmod_empty_dir();
    test_chmod_file_dir();
    test_chmod_nest_dir();
}

// test chmod with file
int test_chmod_file(void) {
    const char *data = testdata;
    int datalen = testdatalen;
    int res;
    int fd;

    start_test("chmod file");

    // create testfile
    unlink(testfile);
    res = create_file(testfile, data, datalen);  // permission 0664
    if (res == -1) {
        PERROR("creat");
        return -1;
    }
    res = check_exist(testfile);
    if (res == -1) {
        PERROR("creat");
        return -1;
    } 
    res = check_mode(testfile, 0644);
    if (res == -1) {
        PERROR("mode");
        return -1;
    }

    // change mode
    res = chmod(testfile, 0755);
    if (res == -1) {
        PERROR("chmod");
        return -1;
    }
    res = check_mode(testfile, 0755);
    if (res == -1) {
        PERROR("chmod");
        return -1;
    }

    // change mode back
    res = chmod(testfile, 0644);
    if (res == -1) {
        PERROR("chmod");
        return -1;
    }
    res = check_mode(testfile, 0644);
    if (res == -1) {
        PERROR("chmod");
        return -1;
    }

    // clean up
    res = unlink(testfile);
    if (res == -1) {
        PERROR("unlink");
        return -1;
    }   
    res = check_nonexist(testfile);
    if (res == -1) {
        PERROR("unlink");
        return -1;
    }

    success();
    return 0;
}

// test chmod with empty dir
int test_chmod_empty_dir(void) {
    int res;

    start_test("chmod empty dir");

    // create dir
    rmdir(testdir);
    res = mkdir(testdir, 0755);
    if (res == -1) {
        PERROR("mkdir");
        return -1;
    }
    res = check_exist(testdir);
    if (res == -1) {
        PERROR("mkdir");
        return -1;
    } 
    res = check_mode(testdir, 0755);
    if (res == -1) {
        PERROR("mode");
        return -1;
    }
 
    // change mode
    res = chmod(testdir, 0644);
    if (res == -1) {
        PERROR("chmod");
        return -1;
    }  
    res = check_mode(testdir, 0644);
    if (res == -1) {
        PERROR("chmod");
        return -1;
    }

    // change mode back
    res = chmod(testdir, 0755);
    if (res == -1) {
        PERROR("chmod");
        return -1;
    }
    res = check_mode(testdir, 0755);
    if (res == -1) {
        PERROR("chmod");
        return -1;
    }

    // clean up
    res = rmdir(testdir);
    if (res == -1) {
        PERROR("rmdir");
        return -1;
    }
    res = check_nonexist(testdir);
    if (res == -1) {
        PERROR("rmdir");
        return -1;
    }

    success();
    return 0;
}

int test_chmod_file_dir(void) {
    char filepath[64];
    const char *data = testdata;
    int datalen = testdatalen;
    int res;

    // path testdir/testfile
    strcpy(filepath, "");
    strcat(filepath, testdir);
    strcat(filepath, "/testfile");

    start_test("chmod dir with file");

    // create dir
    rmdir(testdir);
    res = mkdir(testdir, 0755);
    if (res == -1) {
        PERROR("mkdir");
        return -1;
    } 
    res = check_exist(testdir);
    if (res == -1) {
        PERROR("mkdir");
        return -1;
    } 
    res = check_mode(testdir, 0755);
    if (res == -1) {
        PERROR("mode");
        return -1;
    }

    // create file
    unlink(filepath);
    res = create_file(filepath, data, datalen);
    if (res == -1) {
        PERROR("creat");
        return -1;
    } 
    res = check_exist(filepath);
    if (res == -1) {
        PERROR("creat");
        return -1;
    }     
    res = check_mode(filepath, 0644);
    if (res == -1) {
        PERROR("mode");
        return -1;
    }

    // change file mode
    res = chmod(filepath, 0744) + check_mode(filepath, 0744);
    if (res != 0) {
        PERROR("chmod");
        return -1;
    }
    // check dir mode
    res = check_mode(testdir, 0755);
    if (res == -1) {
        PERROR("dir mode incorrect");
        return -1;
    }

    // chagne dir mode
    res = chmod(testdir, 0777) + check_mode(testdir, 0777);
    if (res != 0) {
        PERROR("chmod");
        return -1;
    }
    // check file mode
    res = check_mode(filepath, 0744);
    if (res == -1) {
        PERROR("file mode incorrect");
        return -1;
    }

    // clean up
    res = unlink(filepath) + rmdir(testdir);
    if (res != 0) {
        PERROR("clean up");
        return -1;
    }
    res = check_nonexist(filepath) + check_nonexist(testdir);
    if (res != 0) {
        PERROR("clean up");
        return -1;
    }

    success();
    return 0;
}

int test_chmod_nest_dir(void) {
    char dirpath[64];
    const char *data = testdata;
    int datalen = testdatalen;
    int res;

    // path testdir/testdir2
    strcpy(dirpath, "");
    strcat(dirpath, testdir);
    strcat(dirpath, "/testdir2");

    start_test("chmod nested dir");

    // create parent dir
    rmdir(testdir);
    res = mkdir(testdir, 0755)
        + check_exist(testdir)
        + check_mode(testdir, 0755);
    if (res != 0) {
        PERROR("mkdir");
        return -1;
    }

    // create child dir
    rmdir(dirpath);
    res = mkdir(dirpath, 0644)
        + check_exist(dirpath)
        + check_mode(dirpath, 0644);
    if (res != 0) {
        PERROR("mkdir");
        return -1;
    }

    // change child mode
    res = chmod(dirpath, 0744) + check_mode(dirpath, 0744);
    if (res != 0) {
        PERROR("chmod");
        return -1;
    }
    // check parent mode
    res = check_mode(testdir, 0755);
    if (res == -1) {
        PERROR("parent mode incorrect");
        return -1;
    }

    // chagne parent mode
    res = chmod(testdir, 0777) + check_mode(testdir, 0777);
    if (res != 0) {
        PERROR("chmod");
        return -1;
    }
    // check child mode
    res = check_mode(dirpath, 0744);
    if (res == -1) {
        PERROR("child mode incorrect");
        return -1;
    }

    // clean up
    res = rmdir(dirpath) + rmdir(testdir);
    if (res != 0) {
        PERROR("clean up");
        return -1;
    }
    res = check_nonexist(dirpath) + check_nonexist(testdir);
    if (res != 0) {
        PERROR("clean up");
        return -1;
    }

    success();
    return 0;
   
}
