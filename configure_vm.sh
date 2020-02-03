#! /bin/bash

### this is just used for configuring the VM

#sudo cat > tmp <<- EOM
#deb http://archive.ubuntu.com/ubuntu bionic main universe multiverse restricted
#deb http://security.ubuntu.com/ubuntu/ bionic-security main multiverse universe restricted
#deb http://archive.ubuntu.com/ubuntu bionic-updates main multiverse universe restricted
#EOM
#
#sudo apt-get update
#sudo apt-get install gcc
#sudo apt-get install g++
#sudo apt-get install cmake
#sudo apt-get install autoconf automake libtool
#
#git clone https://github.com/google/glog
#cd glog
#./autogen.sh
#./configure
#make -j
#sudo make install

git clone https://github.com/google/googletest
cd ../googletest/
mkdir build
cd build
cmake ..
make -j
sudo make install
