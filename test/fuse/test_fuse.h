#define test_open(exist, flags, mode)  do_test_open(exist, flags, #flags, mode)
int test_mknod(void);
int test_mkdir(void);
int test_truncate(int len);
int test_read_seek(void);
int test_write(void);
int test_mkdir_add(void);
int test_readdir(void);
int test_rmdir_unlink(void);
int test_unlink(void);