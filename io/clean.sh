#!/bin/bash

# Clean script for Transport Abstraction Library

echo "Cleaning build artifacts..."

# Remove build directory
if [ -d "build" ]; then
    echo "  Removing build/ directory..."
    rm -rf build
fi

# Remove shared memory files
if [ -d "/dev/shm" ]; then
    echo "  Cleaning shared memory files..."
    rm -f /dev/shm/test_shm* 2>/dev/null || true
fi

echo "Clean completed!"
