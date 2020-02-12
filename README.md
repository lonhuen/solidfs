# cs270

## Get Started

1. Dependencies

  * gcc, g++, make, autoconf, automake, libtool, meso(required)

    * Install
  
      ``` shell
      sudo apt-get update
      sudo apt-get install gcc
      sudo apt-get install g++
      sudo apt-get install cmake
      sudo apt-get install autoconf automake libtool
      sudo apt-get install python3 python3-pip python3-setuptools python3-wheel
    sudo apt-get install ninja-build
      sudo apt-get install pkg-config
    pip3 install meson
      ```
    
    * Add meson to the `PATH`
    
      ``` shell
      # Add meson to PATH
      export PATH=$PATH:~/.local/bin/
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

* Google Test (optional)

  ``` shell
  git clone https://github.com/google/googletest
  cd googletest
  mkdir build
  cd build
  cmake ..
  make
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
        export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/
  ```
      

2. Run Solid File System

``` shell
# creat the mount point
# in build directory
mkdir temp
# run file system
./solidFS
```

3. Run Tests

   open another terminal

   * test_syscall. The corresponding file is `test/fuse/teset_syscall.c`

     (taken from https://github.com/libfuse/libfuse/blob/master/test/test_syscalls.c)

     ``` shell
     # syscall Tests
     sudo ./syscallTest `realpath temp`
     ```

   * Simple read and write test. The corresponding file is `test/fuse/file_operation_test.cpp`

     ``` shell
     sudo ./FuseTests
     ```


## Build from source

If the `glog`, `googletest` or `libfuse` is installed in a self-defined location, by setting Environment Variables

1. GLOG_ROOT
2. GTEST_ROOT
3. FUSE_ROOT

cmake will find the include directories and libraries.

To build from source,

``` shell
mkdir build
cd build
cmake ..
```

## Todo

* `Open` in sys call test sometimes failed `unlink not implemented`
* Add more testcases
* Add Ref
* After fixing the bug in `write_directory`, we have got one new bug in `CoreTests`. Fix it!



## Acknowledgements

- [ ] to be added