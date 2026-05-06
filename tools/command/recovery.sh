#!/bin/bash

./build.sh recovery

rm -f rockdev/recovery.img
cp buildroot/output/rockchip_rk3399_recovery/images/recovery.img    rockdev/

