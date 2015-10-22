#!/bin/sh
SRC=kadai02.c
#WRL=sample/av4.wrl

echo start!!
echo Enter source VRML file!
read WRL
gcc -Wall $SRC
./a.out sample/$WRL.wrl
open image.ppm
echo completed!! "\xF0\x9f\x8d\xbb"
