#!/bin/bash

if [ -f /etc/redhat-release ]; then
    sudo dnf install sqlite-devel-3.42.0-7.fc39.x86_64
elif [ -f /etc/debian_version ]; then
    sudo apt-get install libsqlite3-dev
else
    echo "Unsupported operating system"
    exit 1
fi

rm LOGS
make clean
make all
clear
./server
