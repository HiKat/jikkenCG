#!/bin/sh
SRC=kadai05New.c
#WRL=sample/av5.wrl

echo start!!
echo Enter source VRML file!
read WRL
gcc -Wall $SRC
./a.out sample/$WRL.wrl
open image.ppm
echo completed!! "\xF0\x9f\x8d\xbb"
