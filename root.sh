#!/bin/sh
for((i=5;i<=50;i++));do n=`echo "scale=1; $i * 0.2" | bc`
cd "$n""MeV"
root -l rootelectron.c
root -l rootpositron.c
cd ..;done 