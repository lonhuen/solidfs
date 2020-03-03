# cs270

## Get Started

0. Test Environment

   Instance type: m1.large

   Image name: ubuntu-15.10
   
   Image ID: emi-67a061da

   Architecture: x86_64

1. Dependencies (choose yes for all prompts)
    * gcc, g++, make, autoconf, automake, libtool, meso(required)
      * Install
        ``` shell
        #sudo add-apt-repository ppa:ubuntu-toolchain-r/test
        sudo add-apt-repository ppa:jonathonf/gcc-7.1
        sudo apt-get update
        sudo apt-get install gcc-7 g++-7
        wget https://github.com/Kitware/CMake/releases/download/v3.16.4/cmake-3.16.4-Linux-x86_64.sh
        sudo bash cmake-3.16.4-Linux-x86_64.sh
        sudo apt-get install autoconf automake libtool
        sudo apt-get install python3 python3-pip python3-setuptools python3-wheel
        sudo apt-get install ninja-build
        sudo apt-get install pkg-config
        sudo apt-get install python3-pip
        pip3 install meson
      * Add meson to the `PATH`
        ``` shell
        # Add meson to PATH
        export PATH=$PATH:~/.local/bin/:~/cmake-3.16.4-Linux-x86_64/bin
        ```
      * If there is something wrong with `source.list`, consider replacing the `source.list`
      
        ``` shell
        sudo mv /etc/apt/sources.list /etc/apt/sources.list.backup
        sudo wget https://gist.githubusercontent.com/lonhuen/2a4a5d9992bd831eb74a6b903107c927/raw/53dc53adac5faed5335072a11b5a30d7d41578a6/source.list.cs270 -O /etc/apt/sources.list
        ```
    * Google Log (required)
      ``` shell
      git clone https://github.com/google/glog
      cd glog
      ./autogen.sh
      ./configure
      make -j
      sudo make install
      ```
     
    * libFuse 3 (required)
      * Installation
        ``` shell
        git clone https://github.com/libfuse/libfuse.git
        cd libfuse
        mkdir build
        cd build
        meson ..
        ninja
        pip3 install pytest
        sudo python3 -m pytest test/
        sudo ninja install
        ```
      * Configuration
        * Open the file `/usr/local/etc/fuse.conf` and append `user_allow_other`
          ``` shell
          # The file /etc/fuse.conf allows for the following parameters:
          #
          # user_allow_other - Using the allow_other mount option works fine as root, in
          # order to have it work as user you need user_allow_other in /etc/fuse.conf as
          # well. (This option allows users to use the allow_other option.) You need
          # allow_other if you want users other than the owner to access a mounted fuse.
          # This option must appear on a line by itself. There is no value, just the
          # presence of the option.
          user_allow_other
          ```
        * Library Path
        
          ``` shell
          export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/:/usr/local/lib/x86_64-linux-gnu/

2. Installation
   ``` shell
   git clone https://github.com/lonhuen/cs270.git
   cd cs270
   ```
   
   * Google Test: comment out this line in `cs270/CMakeLists.txt` (line 11)
   ``` cmake
   set(ENABLE_TEST true)
   ```

3. Run Solid File System (in build directory)

    ``` shell
    # creat the mount point in build directory
    cd build
    mkdir temp
    # run file system
    ./solidFS temp <nr_block> <nr_iblock>
    ```
    An example command to run the file system is
    
    ```shell
    ./solidFS temp 262666 9
    ```

4. Run Tests (optional - all in build directory)

   open another terminal

   * test_syscall. The corresponding file is `test/fuse/teset_syscall.c`

     ``` shell
     # syscall Tests
     cd build
     sudo ./syscallTest `realpath temp`
     ```
      test cases in test_syscall:
      1. test_mknod: test mknod 
      2. test_mkdir: test mkdir
      3. test_open: test open
        (above three taken from https://github.com/libfuse/libfuse/blob/master/test/test_syscalls.c)
      4. test_read_seek:
          - read from *start* of file for *size* bytes <= *filesize*
          - seek to *offset* < *filesize*, read *size* bytes, where *offset* + *size* <= *filesize*
          - seek to *offset* < *filesize*, read *size* bytes, where *offset* + *size* > *filesize*
          - seek to *offset* > *filesize*
      5. test_write:
          - create and write a file
          - open an existing file, write from start of file
          - TODO: add test case to open an existing file, write from *offset* of file
      6. test_mkdir_add:
          - create a directory
          - create nested directories: mkdir("testdir2/testdir"), where both *testdir2* and *testdir* not exists.
          - create nested directories: mkdir("testdir2"), mkdir("testdir2/testdir")
          - TODO: need to fix unlink --> occasionlly, there may be `unlink not implemented` error. When this occurs, there will be *testfile* file remained in the temp/ folder, which leads to `mkdir() - file exists` error in this test case. 
      7. test_readdir:
          - create the following structure

            testdir/\
            |------ testdir/ (empty dir)\
            |------ testdir2/ (contains testfile2) \
            |------ testfile (a regular file)
          - read testdir to check entry number, and file content
      8. test_rmdir: 
          - remove empty directory
          - remove directory with file
          - remove directory with sub directory
          - remove directory with sub directory, where the sub directory has files in it

   * Simple read and write test. The corresponding file is `test/fuse/file_operation_test.cpp`

     ``` shell
     cd build
     sudo ./FuseTests
     ```
   * Unit tests.
   
     ``` shell
     cd build
     ./CoreTests
     ```




## Todo

* `Open` in sys call test sometimes failed `unlink not implemented`
* Add more testcases
* Add Ref
* After fixing the bug in `write_directory`, we have got one new bug in `CoreTests`. Fix it!



## Acknowledgements

- [ ] to be added
