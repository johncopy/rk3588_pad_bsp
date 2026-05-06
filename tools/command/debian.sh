#!/bin/bash

./build.sh debian

rm -f rockdev/rootfs.img
cp out/linaro-rootfs.img   rockdev/rootfs.img

