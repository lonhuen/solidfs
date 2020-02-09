# cs270

# Compilation

1. Dependencies

  * glog

    ``` shell
    git clone https://github.com/google/glog
    cd glog
    ./autogen.sh
    ./configure
    make -j
    sudo make instal
    ```

  * gtest

    ``` shell
    git clone https://github.com/google/googletest
    cd googletest
    mkdir build
    cd build
    cmake ..
    make
    make install
    ```


2. Compilation

   ``` shell
   mkdir build
   cd build
   cmake ..
   make -j
   ```

3. Run Tests

   ``` shell
   # creat the mount point
   mkdir -p temp
   
   # run file system
   ./solidFS
   
   # unit test
   ./CoreTests
   
   # Fuse Tests
   sudo ./FuseTests
   
   # syscall Tests
   sudo ./syscallTest `realpath temp`
   ```

4. Some Todo-s

   * `Open` in sys call test sometimes failed `unlink not implemented`
   * Add more testcases
   * Add Ref