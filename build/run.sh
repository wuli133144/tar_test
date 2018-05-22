#!/bin/bash


rm -rf CMakeC* CMakeF* cmake_*
cmake ..

make 
if [ $? -eq 0 ];then
  cp -R  sunlands_TEST   ../ 

fi 
