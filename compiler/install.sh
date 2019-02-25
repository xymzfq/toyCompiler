#!/bin/bash
if [ X$1 = X ] ; then
cd c
make
cd ..
cd psc
make
cd ..
cd js
make

else
cd c
make clean
cd .. 
cd psc
make clean
cd ..
cd js
make clean

fi
