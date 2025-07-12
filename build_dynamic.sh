#!/bin/bash

# Build script for Dynamic MAPF components

echo "Building Dynamic MAPF components..."

# Check if required libraries are available
if ! pkg-config --exists eigen3; then
    echo "Warning: Eigen3 not found via pkg-config"
fi

if ! pkg-config --exists boost; then
    echo "Warning: Boost not found via pkg-config"
fi

# Compile the dynamic components
g++ -std=c++17 -I./inc -O3 -DNDEBUG \
    -o dynamic_lns \
    src/dynamic_driver.cpp \
    src/DynamicInstance.cpp \
    src/Instance.cpp \
    src/common.cpp \
    src/LNS.cpp \
    src/BasicLNS.cpp \
    src/InitLNS.cpp \
    src/PathTable.cpp \
    src/SingleAgentSolver.cpp \
    src/SpaceTimeAStar.cpp \
    src/SIPP.cpp \
    src/ReservationTable.cpp \
    src/ConstraintTable.cpp \
    src/AnytimeBCBS.cpp \
    src/AnytimeEECBS.cpp \
    src/CBS/CBS.cpp \
    src/CBS/CBSNode.cpp \
    src/CBS/Conflict.cpp \
    src/CBS/CBSHeuristic.cpp \
    src/CBS/ECBS.cpp \
    src/CBS/GCBS.cpp \
    src/CBS/MDD.cpp \
    src/CBS/CorridorReasoning.cpp \
    src/CBS/RectangleReasoning.cpp \
    src/CBS/MutexReasoning.cpp \
    src/CBS/IncrementalPairwiseMutexPropagation.cpp \
    src/CBS/ConstraintPropagation.cpp \
    src/CBS/PBS.cpp \
    src/PIBT/graph.cpp \
    src/PIBT/grid.cpp \
    src/PIBT/mapf.cpp \
    src/PIBT/node.cpp \
    src/PIBT/pibt_agent.cpp \
    src/PIBT/pibt.cpp \
    src/PIBT/pps.cpp \
    src/PIBT/problem.cpp \
    src/PIBT/simplegrid.cpp \
    src/PIBT/solver.cpp \
    src/PIBT/task.cpp \
    src/PIBT/winpibt.cpp \
    -lboost_system -lboost_filesystem -lboost_program_options -lpthread

if [ $? -eq 0 ]; then
    echo "Build successful! Dynamic MAPF executable created: dynamic_lns"
    echo ""
    echo "Usage examples:"
    echo "  ./dynamic_lns --help"
    echo "  ./dynamic_lns -k 10 -t 30 -s 2.0 -i 3.0"
    echo "  ./dynamic_lns -m warehouse-20-40-10-2-2.map -k 20 -t 60 -w true"
else
    echo "Build failed!"
    exit 1
fi 