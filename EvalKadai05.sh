#!/bin/sh
SRC=kadai05New.c

WRL=sample/av5.wrl

PPM0=Kadai05ForAv5-0.ppm
CAMERA0=0.0

PPM1=Kadai05ForAv5-1.ppm
CAMERA1=50.0

PPM2=Kadai05ForAv5-2.ppm
CAMERA2=-50.0

gcc -Wall $SRC
./a.out $WRL $PPM0 $CAMERA0
open $PPM0

./a.out $WRL $PPM1 $CAMERA1
open $PPM1

./a.out $WRL $PPM2 $CAMERA2
open $PPM2
echo completed!! "\xF0\x9f\x8d\xbb"
