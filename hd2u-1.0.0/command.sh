#!/bin/sh
for((i=10;i<=15;i++));do n=`echo "scale=1; $i * 0.2" | bc`
nn="$n"
tail="MeV"
filename=$nn$tail
cd $filename
./TestEm14 run1.mac;done 