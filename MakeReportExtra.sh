#!/bin/sh
SRC=ReportForKadaiExtra

echo start!!
platex $SRC.tex
dvipdfmx $SRC.dvi
open $SRC.pdf
echo completed!!
echo "\xF0\x9f\x8d\xbb" ===================================