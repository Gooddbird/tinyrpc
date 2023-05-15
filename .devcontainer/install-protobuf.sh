#!/usr/bin/env bash

echo "Building protobuf"

sudo apt-get install autoconf automake libtool curl make g++ unzip
git clone https://github.com/google/protobuf --depth 1 -b ${PROTOBUF_BRANCH}
cd protobuf
./autogen.sh
./configure --prefix=/usr
make -j 3
make check
sudo make install
sudo ldconfig # refresh shared library cache.
cd ..
rm -rf protobuf