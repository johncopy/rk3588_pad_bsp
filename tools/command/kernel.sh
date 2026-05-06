#!/bin/bash

./build.sh kernel

mkdir -p rockdev/

rm -f rockdev/kernel.img
rm -f rockdev/resource.img
rm -f rockdev/boot.img

cp kernel/kernel.img 	rockdev/
cp kernel/resource.img 	rockdev/
cp kernel/boot.img 		rockdev/
