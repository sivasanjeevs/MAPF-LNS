#!/bin/bash

# Script to run dynamic MAPF with warehouse scenario
# Usage: ./run_warehouse_dynamic.sh [num_agents] [simulation_time]

# Default parameters
NUM_AGENTS=${1:-100}  # Default to 100 agents
SIM_TIME=${2:-60}     # Default to 60 seconds simulation time

echo "Running Dynamic MAPF with Warehouse Scenario"
echo "Map: warehouse-20-40-10-2-2.map"
echo "Scenario: warehouse-20-40-10-2-2-10000agents-1.scen"
echo "Number of agents: $NUM_AGENTS"
echo "Simulation time: $SIM_TIME seconds"
echo ""

# Check if files exist
if [ ! -f "instances/warehouse-20-40-10-2-2.map" ]; then
    echo "Error: Map file not found!"
    exit 1
fi

if [ ! -f "instances/warehouse-20-40-10-2-2-10000agents-1.scen" ]; then
    echo "Error: Scenario file not found!"
    exit 1
fi

# Build the dynamic executable if it doesn't exist
if [ ! -f "dynamic_driver" ]; then
    echo "Building dynamic driver..."
    ./build_dynamic.sh
    if [ $? -ne 0 ]; then
        echo "Build failed!"
        exit 1
    fi
fi

# Run the dynamic simulation
echo "Starting dynamic simulation..."
./dynamic_driver \
    --map instances/warehouse-20-40-10-2-2.map \
    --scenario instances/warehouse-20-40-10-2-2-10000agents-1.scen \
    --agents $NUM_AGENTS \
    --time $SIM_TIME \
    --output warehouse_dynamic_paths.txt \
    --visualize

echo ""
echo "Simulation completed!"
echo "Results saved to: warehouse_dynamic_paths.txt"
echo "Visualization saved to: warehouse_dynamic_visualization.html" 