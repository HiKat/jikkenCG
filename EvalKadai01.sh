#!/bin/sh
SRC=kadai01.c

echo start!!
rm image.ppm
gcc -Wall $SRC
./a.out
open image.ppm
echo completed!! "\xF0\x9f\x8d\xbb"
