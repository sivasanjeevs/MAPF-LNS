#!/bin/bash

MAP_FILE="warehouse-20-40-10-2-2.map"
MAP_URL="https://movingai.com/benchmarks/warehouse/$MAP_FILE"

if [ -f "$MAP_FILE" ]; then
    echo "$MAP_FILE already exists."
else
    echo "Downloading $MAP_FILE ..."
    wget "$MAP_URL" -O "$MAP_FILE"
    if [ $? -eq 0 ]; then
        echo "Download complete."
    else
        echo "Failed to download $MAP_FILE."
        exit 1
    fi
fi 