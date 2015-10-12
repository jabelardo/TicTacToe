#!/bin/bash

mkdir -p ../build

pushd ../build

CommonFlags="-Wall -Werror -Wno-unused-variable -Wno-unused-function -Wno-writable-strings \
	-std=c++11 -fno-rtti -fno-exceptions -DBUILD_INTERNAL=1 -DBUILD_SLOW=1 \
	-DBUILD_OSX=1 -framework SDL2"

c++ $CommonFlags ../src/main.cpp -o tic-tac-toe -g 

popd
