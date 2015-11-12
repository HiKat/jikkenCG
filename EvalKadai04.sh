#!/bin/sh
SRC=kadai04.c
WRL=sample/av5.wrl
PPM=Kadai04ForAv5.ppm

gcc -Wall $SRC
./a.out $WRL $PPM
open $PPM
echo completed!! "\xF0\x9f\x8d\xbb"
