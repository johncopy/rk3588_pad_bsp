#!/bin/bash

./build.sh uboot

mkdir -p rockdev/
rm -f rockdev/MiniLoaderAll.bin
cp u-boot/*_loader_*.bin  rockdev/MiniLoaderAll.bin
rm -f rockdev/uboot.img
cp u-boot/uboot.img       rockdev/
rm -f rockdev/trust.img
cp u-boot/trust.img       rockdev/
