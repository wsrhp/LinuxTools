#!/bin/sh
for((i=1;i<=4;i++));do n=`echo "scale=1; $i * 0.2" | bc`
cd "0""$n""MeV"
./TestEm14 run1.mac
cd ..;done 