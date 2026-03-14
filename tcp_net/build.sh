#!/bin/bash

# Build script for aw_s24cn_network library

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Building tcp_network_lib...${NC}"

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake ..

# Build the project
echo -e "${YELLOW}Building project...${NC}"
make -j$(nproc)

echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${YELLOW}Library location: ${NC}build/lib/"
echo -e "${YELLOW}Example binary: ${NC}build/bin/enhanced_tcp_example"
echo -e "${YELLOW}Headers: ${NC}src/"

# Optionally run the example
if [ "$1" = "--run-example" ]; then
    echo -e "${YELLOW}Running example...${NC}"
    ./bin/enhanced_tcp_example
fi
