#!/bin/sh
SRC=kadai04-2.c

WRL1=sample/av5.wrl
PPM1=Kadai04ForAv5-1.ppm
CAMERA1=-10.0


PPM2=Kadai04ForAv5-2.ppm
CAMERA2=10.0

echo start!!
gcc -Wall $SRC

./a.out $WRL1 $PPM1 $CAMERA1
open $PPM1

./a.out $WRL1 $PPM2 $CAMERA2
open $PPM2


echo completed!! "\xF0\x9f\x8d\xbb" ========================================
