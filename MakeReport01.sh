#!/bin/sh
SRC=ReportForKadai01

echo start!!
platex $SRC.tex
dvipdfmx $SRC.dvi
open $SRC.pdf
echo completed!!
echo "\xF0\x9f\x8d\xbb"
