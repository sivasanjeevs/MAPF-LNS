#!/bin/bash

echo "Building Debug Dynamic MAPF Test..."

# Compile the debug version with more detailed error checking
g++ -std=c++17 -I./inc -I./inc/CBS -I./inc/PIBT -g -DDEBUG \
    -o debug_dynamic \
    debug_dynamic.cpp \
    src/DynamicInstance.cpp \
    src/Instance.cpp \
    src/common.cpp \
    -lboost_system -lboost_filesystem -lboost_program_options -lpthread

if [ $? -eq 0 ]; then
    echo "Debug build successful!"
    echo "Run with: ./debug_dynamic"
else
    echo "Debug build failed!"
    exit 1
fi 