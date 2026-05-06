#!/bin/bash

# 等待网络接口就绪
sleep 1

# 等待 wlan0 接口就绪
echo "Waiting for wlan0 interface..."
while [ ! -e /sys/class/net/wlan0 ]; do
    echo "wlan0 interface not found, waiting..."
    sleep 1
done
echo "wlan0 interface is ready"

# 等待 dummy0 接口就绪，如果不存在则创建
echo "Waiting for dummy0 interface..."
while [ ! -e /sys/class/net/dummy0 ]; do
    echo "dummy0 interface not found, waiting..."
    sleep 1
done
echo "dummy0 interface is ready"

# 获取 wlan0 的 MAC 地址
WLAN_MAC=$(cat /sys/class/net/wlan0/address)

# 设置 dummy0 的 MAC 地址
ip link set dummy0 address $WLAN_MAC
echo "dummy0 MAC address set to $WLAN_MAC"
