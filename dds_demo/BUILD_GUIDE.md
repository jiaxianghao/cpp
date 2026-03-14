# Build Guide

Complete build instructions for the FastDDS Wrapper project.

## Prerequisites

### Required

- **CMake** 3.16 or higher
- **C++ Compiler** with C++17 support:
  - GCC 7+ (Linux)
  - Clang 5+ (macOS/Linux)
  - MSVC 2017+ (Windows)
- **FastDDS** 2.10 or higher
- **FastDDS-Gen** code generator tool

### Optional

- **Python 3** (for IDL generator tool)
- **Git** (for version control)

## Installing Dependencies

### Ubuntu/Debian

```bash
# Update package list
sudo apt update

# Install build tools
sudo apt install -y cmake g++ git

# Install FastDDS and FastCDR
sudo apt install -y libfastrtps-dev libfastcdr-dev

# Install FastDDS-Gen (manual installation required)
# See: https://github.com/eProsima/Fast-DDS-Gen
```

### macOS

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake
brew install fastrtps

# Install FastDDS-Gen manually
```

### Windows

1. Install Visual Studio 2017 or later
2. Install CMake from https://cmake.org/download/
3. Install FastDDS from source or binary packages
4. Install FastDDS-Gen from GitHub

## Building from Source

### Step 1: Clone/Download the Project

```bash
cd dds_demo
```

### Step 2: Create Build Directory

```bash
mkdir build
cd build
```

### Step 3: Configure with CMake

```bash
cmake ..
```

**Options:**

```bash
# Specify build type
cmake -DCMAKE_BUILD_TYPE=Release ..

# Custom FastDDS installation path
cmake -DCMAKE_PREFIX_PATH=/path/to/fastdds/install ..

# Verbose output
cmake -DCMAKE_VERBOSE_MAKEFILE=ON ..
```

### Step 4: Build

```bash
# Linux/macOS
make

# Or use CMake build command (cross-platform)
cmake --build .

# Build in parallel (faster)
make -j$(nproc)  # Linux
make -j$(sysctl -n hw.ncpu)  # macOS

# Windows (Visual Studio)
cmake --build . --config Release
```

### Step 5: Verify Build

```bash
# Check if library was built
ls libdds_wrapper.a  # or .lib on Windows

# Check if examples were built
ls bin/examples/

# Run a test
./bin/examples/basic_pubsub
```

## Build Targets

### All Targets

```bash
make all  # Build everything
```

### Specific Targets

```bash
# Build library only
make dds_wrapper

# Build IDL types only
make idl_types

# Build specific example
make basic_pubsub

# Build tools
make list_types
```

### Clean Build

```bash
# Clean build artifacts
make clean

# Complete rebuild
rm -rf build
mkdir build && cd build
cmake .. && make
```

## Installation

### System-Wide Installation

```bash
cd build
sudo make install
```

Default installation paths:
- Headers: `/usr/local/include/dds_wrapper/`
- Library: `/usr/local/lib/libdds_wrapper.a`
- Examples: `/usr/local/bin/examples/`
- Tools: `/usr/local/bin/tools/`

### Custom Installation Path

```bash
cmake -DCMAKE_INSTALL_PREFIX=/opt/dds_wrapper ..
make
sudo make install
```

## CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | Release | Debug, Release, RelWithDebInfo, MinSizeRel |
| `CMAKE_INSTALL_PREFIX` | /usr/local | Installation directory |
| `CMAKE_PREFIX_PATH` | (empty) | FastDDS installation path |
| `CMAKE_CXX_STANDARD` | 17 | C++ standard version |

## Project Structure

After building:

```
dds_demo/
├── build/
│   ├── libdds_wrapper.a        # Static library
│   ├── bin/
│   │   ├── examples/           # Example executables
│   │   └── tools/              # Utility tools
│   └── idl/                    # Generated IDL code
├── include/dds_wrapper/        # Header files
├── src/                        # Source files
├── idl/                        # IDL definitions
├── config/                     # Configuration templates
├── examples/                   # Example source
├── tools/                      # Tool scripts
└── docs/                       # Documentation
```

## Troubleshooting

### CMake Cannot Find FastDDS

```bash
# Solution 1: Install system-wide
sudo apt install libfastrtps-dev  # Ubuntu

# Solution 2: Specify path
cmake -DCMAKE_PREFIX_PATH=/path/to/fastdds ..

# Solution 3: Set environment variable
export CMAKE_PREFIX_PATH=/path/to/fastdds
cmake ..
```

### fastddsgen Not Found

The project will build without fastddsgen, but you won't be able to add new IDL files.

To install FastDDS-Gen:

```bash
git clone --recursive https://github.com/eProsima/Fast-DDS-Gen.git
cd Fast-DDS-Gen
./gradlew assemble
sudo cp scripts/fastddsgen /usr/local/bin/
sudo chmod +x /usr/local/bin/fastddsgen
```

### Compiler Errors

Make sure you have C++17 support:

```bash
# Check GCC version
g++ --version  # Should be 7.0 or higher

# Check Clang version
clang++ --version  # Should be 5.0 or higher

# Force specific compiler
cmake -DCMAKE_CXX_COMPILER=g++-9 ..
```

### Linking Errors

```bash
# Missing FastDDS libraries
sudo ldconfig  # Update library cache (Linux)

# Check library path
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/fastdds/lib
```

## Using the Library in Your Project

### Method 1: System Installation

After `sudo make install`:

```cmake
# Your CMakeLists.txt
find_package(fastrtps REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app dds_wrapper fastrtps fastcdr)
target_include_directories(my_app PUBLIC /usr/local/include)
```

### Method 2: Subdirectory

```cmake
# Your CMakeLists.txt
add_subdirectory(path/to/dds_demo)

add_executable(my_app main.cpp)
target_link_libraries(my_app dds_wrapper)
```

### Method 3: Find Package

```cmake
# Your CMakeLists.txt
find_package(DDSWrapper REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app DDSWrapper::dds_wrapper)
```

## Development Builds

### Debug Build

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

Features:
- Debug symbols included
- Assertions enabled
- No optimizations
- Larger binaries

### Release Build

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

Features:
- Optimized for speed
- No debug symbols
- Smaller binaries
- Best for production

### Release with Debug Info

```bash
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make
```

Features:
- Optimizations enabled
- Debug symbols included
- Good for profiling

## Cross-Compilation

Example for ARM:

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=arm-toolchain.cmake ..
make
```

## Continuous Integration

Example GitHub Actions workflow:

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: sudo apt install -y cmake g++ libfastrtps-dev
      - name: Build
        run: |
          mkdir build && cd build
          cmake ..
          make
      - name: Test
        run: ./build/bin/examples/basic_pubsub
```

## Performance Optimization

### Compiler Flags

```bash
# Maximum optimization
cmake -DCMAKE_CXX_FLAGS="-O3 -march=native" ..

# Link-time optimization
cmake -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON ..
```

### Build Parallel

```bash
# Use all CPU cores
make -j$(nproc)
```

## Summary

Quick start:

```bash
mkdir build && cd build
cmake ..
make
./bin/examples/basic_pubsub
```

For more information, see the [Quick Start Guide](docs/QUICK_START.md).
