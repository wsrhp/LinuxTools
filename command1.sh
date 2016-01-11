#!/bin/sh
for((i=5;i<=25;i++));do n=`echo "scale=1; $i * 0.2" | bc`
cd "$n""MeV"
./TestEm14 run1.mac
cd ..;done 