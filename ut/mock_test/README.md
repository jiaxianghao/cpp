# Mock Test Project

This project contains mock implementations and unit tests for UOS RMAP functionality.

## Project Structure

- `CMakeLists.txt` - CMake configuration file
- `mock_lib/` - Mock library implementations
  - `uos_rmap_mock.h` - Mock header for UOS RMAP functionality
- `ut/` - Unit test files
  - `uos_rmap_mock_test.cc` - Unit tests for mock functionality

## Building the Project

```bash
mkdir build
cd build
cmake ..
make
```

## Running Tests

After building, you can run the tests from the build directory.

## Contributing

Please ensure all code follows the project's coding standards and includes appropriate tests.
