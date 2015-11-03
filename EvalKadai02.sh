#!/bin/sh
SRC=kadai02.c

WRL1=sample/av1.wrl
PPM1=Kadai02ForAv1.ppm

WRL2=sample/av2.wrl
PPM2=Kadai02ForAv2.ppm

WRL3=sample/av3.wrl
PPM3=Kadai02ForAv3.ppm

WRL4=sample/av4.wrl
PPM4=Kadai02ForAv4.ppm

WRLhead=sample/head.wrl
PPMhead=Kadai02ForHead.ppm

WRL1997=sample/iiyama1997.wrl
PPM1997=Kadai02ForIiyama1997.ppm


echo start!!
gcc -Wall $SRC
./a.out $WRL1 $PPM1
open $PPM1

./a.out $WRL2 $PPM2
open $PPM2

./a.out $WRL3 $PPM3
open $PPM3

./a.out $WRL4 $PPM4
open $PPM4

./a.out $WRLhead $PPMhead
open $PPMhead

./a.out $WRL1997 $PPM1997
open $PPM1997

echo completed!! "\xF0\x9f\x8d\xbb"
