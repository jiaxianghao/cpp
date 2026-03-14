#!/bin/bash

# Build script for Transport Abstraction Library

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Transport Abstraction Library Build Script${NC}"
echo "=========================================="

# Check if cmake is installed
if ! command -v cmake &> /dev/null
then
    echo -e "${RED}Error: cmake is not installed${NC}"
    echo "Please install cmake: sudo apt-get install cmake"
    exit 1
fi

# Create build directory
if [ -d "build" ]; then
    echo -e "${YELLOW}Cleaning existing build directory...${NC}"
    rm -rf build
fi

echo "Creating build directory..."
mkdir -p build
cd build

# Configure
echo -e "${GREEN}Configuring project with CMake...${NC}"
cmake ..

# Build
echo -e "${GREEN}Building project...${NC}"
make -j$(nproc)

echo ""
echo -e "${GREEN}Build completed successfully!${NC}"
echo ""
echo "Output files:"
echo "  - Static library: build/libtransport.a"
echo "  - Shared library: build/libtransport.so"
echo "  - Example program: build/transport_example"
echo ""
echo "To run the example:"
echo "  cd build && ./transport_example"
echo ""
echo "To install the library:"
echo "  cd build && sudo make install"
