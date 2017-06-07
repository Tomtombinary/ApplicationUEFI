#!/bin/bash
tunctl -t tap0
ifconfig tap0 192.168.0.1 netmask 255.255.255.0 up

# activate ip forwarding
echo 1 > /proc/sys/net/ipv4/ip_forward

# Create forwarding rules, where
# tap0 - virtual interface
# eth0 - net connected interface

iptables -A FORWARD -i tap0 -o wlan0 -j ACCEPT
iptables -A FORWARD -i wlan0 -o tap0 -m state --state ESTABLISHED,RELATED -j ACCEPT
iptables -t nat -A POSTROUTING -o wlan0 -j MASQUERADE


