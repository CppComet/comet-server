#!/bin/bash
 
echo "Build jwt-cpp..."
cd dependencies/jwt-cpp
cmake .
make
echo "Build jwt-cpp complete"