#!/bin/bash

cd build
/usr/bin/cmake --build /home/jonsnow/cpplox/build --config Debug --target all -j 22 --
if make; then
    ./cpplox
fi


