#!/bin/bash
#
git clone git://git.code.sf.net/p/tinyxml/git tinyxml
cd tinyxml
make -j $NUM_COMPILE_JOBS
ar cr libtinyxml.a *o
sudo cp libtinyxml.a /usr/lib/
sudo mkdir /usr/include/tinyxml
sudo cp *.h /usr/include/tinyxml