#!/bin/sh
for((i=10;i<=15;i++));do n=`echo "scale=1; $i * 0.2" | bc`
echo "$n""MeV";done 