#!/bin/sh

test -d build || ./build.sh
cd build/src/
rm -f *.svg *.txt && make && mpirun -v -N 100 ./exec
