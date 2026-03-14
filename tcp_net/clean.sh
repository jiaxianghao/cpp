#!/bin/bash

# Clean script for aw_s24cn_network library

set -e

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Cleaning tcp_network_lib...${NC}"

# Remove build directory
if [ -d "build" ]; then
    echo -e "${YELLOW}Removing build directory...${NC}"
    rm -rf build
fi

# Remove any other generated files
echo -e "${YELLOW}Cleaning completed!${NC}"
