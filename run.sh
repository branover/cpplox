#!/bin/bash

cd build
/usr/bin/cmake /home/jonsnow/cpplox/build
/usr/bin/cmake --build /home/jonsnow/cpplox/build --config Debug --target all -j 22 --
if [ $? -eq 0 ]; then
    cd ../
    ./build/cpplox $1
fi


