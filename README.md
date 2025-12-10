# ShaderGraph

A sample CMake C++ project.

## Building

### Prerequisites
- CMake 3.16 or higher
- C++17 compatible compiler

### Build Instructions

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Run
./bin/ShaderGraph
```

### Project Structure

```
shaderGraph/
├── CMakeLists.txt      # CMake configuration
├── README.md           # This file
├── include/            # Header files
│   └── app.h
├── src/                # Source files
│   ├── main.cpp
│   └── app.cpp
└── build/              # Build output (generated)
```
