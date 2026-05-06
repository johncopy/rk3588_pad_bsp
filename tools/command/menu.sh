#!/bin/bash

cd kernel
make ARCH=arm64 menuconfig
cd ..
