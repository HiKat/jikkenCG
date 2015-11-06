#!/bin/sh
SRC=kadai03.c

WRL4=sample/av4.wrl
PPM4=Kadai03ForAv4.ppm

WRLhead=sample/head.wrl
PPMhead=Kadai03ForHead.ppm

WRL1997=sample/iiyama1997.wrl
PPM1997=Kadai03ForIiyama1997.ppm


echo start!!
gcc -Wall $SRC

./a.out $WRL4 $PPM4
open $PPM4

./a.out $WRLhead $PPMhead
open $PPMhead

./a.out $WRL1997 $PPM1997
open $PPM1997

echo completed!! "\xF0\x9f\x8d\xbb"
