#!/bin/bash

# Check if lns executable exists
if [ ! -f "./lns" ]; then
    echo "Error: lns executable not found!"
    echo "Please make sure the lns executable is in the current directory."
    exit 1
fi

echo "Using existing lns executable for dynamic MAPF system..."

echo "Build successful! Starting dynamic visualization..."

# Run the dynamic visualizer
# You can start with initial agents or start empty
if [ "$1" = "--with-initial" ]; then
    # Start with initial agents
    python3 dynamic_visualizer.py random-32-32-20.map random-32-32-20-random-1.scen 9
else
    # Start empty and add agents dynamically
    python3 dynamic_visualizer.py random-32-32-20.map
fi 