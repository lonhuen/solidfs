# cs270

## Get Started

0. Test Environment

   * Instance type: m1.large
   * Image distribute: ubuntu-15.10
   * Image ID: emi-67a061da
   * Architecture: x86_64

1. Dependencies
   Choose "yes" or "y" for all prompt, ignore error message in the terminal output. 
    
    * gcc, g++, make, autoconf, automake, libtool, meso
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
      * If there is something wrong with `source.list`, consider replacing the `source.list` and run all previous commands
      
        ``` shell
        sudo mv /etc/apt/sources.list /etc/apt/sources.list.backup
        sudo wget https://gist.githubusercontent.com/lonhuen/2a4a5d9992bd831eb74a6b903107c927/raw/53dc53adac5faed5335072a11b5a30d7d41578a6/source.list.cs270 -O /etc/apt/sources.list
        ```
   
    * Google Log
      ``` shell
      git clone https://github.com/google/glog
      cd glog
      ./autogen.sh
      ./configure
      make -j
      sudo make install
      ```
     
    * libFuse 3
      * Installation
      libFuse 3 should be installed in ```/home/ubuntu```, run ```cd``` command if directly followed from previous step.
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
        * Open the file `/usr/local/etc/fuse.conf` (sudo is necessary to edit a readonly file), 
          ``` shell
          sudo vim /usr/local/etc/fuse.conf
          ```
          and append `user_allow_other`
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
          ```

2. Installation
   The repo should be installed in ```/home/ubuntu```, run ```cd``` command if directly followed from previous step.
   ``` shell
   git clone https://github.com/lonhuen/cs270.git
   ```
   
   * Google Test: comment out this line in `cs270/CMakeLists.txt` (line 11)
   ``` cmake
   set(ENABLE_TEST true)
   ```

3. Run Solid File System (in cs270/build directory)

    ``` shell
    # create build directory
    cd cs270
    mkdir build
    # creat the mount point in build directory
    cd build
    mkdir <mount_pt>
    # make file system
    cmake ..
    make
    # run file system
    ./solidFS <mount_pt> <nr_block> <nr_iblock>
    ```
    An example command to run the file system is under cs270/build
    
    ```shell
    mkdir temp
    cmake ..
    make
    ./solidFS temp 262666 9
    ```

4. Run Tests (optional - all in cs270/build directory)

   open another terminal

   * test_syscall. The corresponding files are in `cs270/test/fuse/`

     ``` shell
     # syscall Tests
     ./syscallTest `realpath <mount_pt>`
     ```
      
   * Simple read and write test. The corresponding file is `cs270/test/fuse/file_operation_test.cpp`

     ``` shell
     cd build
     ./FuseTests `realpath <mount_pt>`
     ```
   * Unit tests.
   
     ``` shell
     cd build
     ./CoreTests `realpath <mount_pt>`
     ```

## Acknowledgements

- [ ] to be added
