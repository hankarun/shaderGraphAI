# ShaderGraph

A visual node-based shader editor built with C++, OpenGL, ImGui, and ImNodeFlow. Create fragment shaders by connecting nodes in an intuitive graph interface.

![ShaderGraph Editor](img/Screenshot%202025-12-10%20at%2022.30.27.png)

## Features

- **Visual Node Graph**: Build shaders by connecting nodes instead of writing code
- **Real-time Preview**: See shader changes instantly
- **Multiple Node Types**:
  - **Input Nodes**: Time, Position (UV), Normal
  - **Math Nodes**: Add, Subtract, Multiply, Divide, Sin, Cos, Abs, Fract, Mix, Step, Smoothstep
  - **Vector Nodes**: Color (RGB), Float constant, Split, Combine
  - **Output Node**: Final fragment color output
- **GLSL Code Generation**: Automatically generates fragment shader code from the node graph

## Building

### Prerequisites
- CMake 3.16 or higher
- C++17 compatible compiler
- OpenGL 3.3+ support

### Build Instructions

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build . -j4

# Run
./bin/ShaderGraph
```

## Usage

1. **Add Nodes**: Right-click on empty space to open the node menu
2. **Connect Nodes**: Drag from an output pin to an input pin to create connections
3. **Edit Values**: Adjust node parameters (colors, floats) directly in the node
4. **Delete Nodes**: Right-click on a node and select "Delete Node"
5. **View Shader**: The generated GLSL code updates in real-time

## Project Structure

```
shaderGraph/
├── CMakeLists.txt          # CMake configuration
├── README.md               # This file
├── img/                    # Screenshots
├── include/                # Header files
│   ├── app.h               # Application class
│   ├── mat.h               # Math utilities
│   ├── shader_graph.h      # Shader graph editor
│   └── shader_nodes.h      # Node definitions
├── src/                    # Source files
│   ├── main.cpp            # Entry point
│   ├── app.cpp             # Application implementation
│   └── mat.cpp             # Math implementation
├── ImNodeFlow-master/      # Node graph library
└── build/                  # Build output (generated)
```

## Dependencies

All dependencies are fetched automatically via CMake:
- [GLFW](https://www.glfw.org/) - Window and input handling
- [Dear ImGui](https://github.com/ocornut/imgui) - Immediate mode GUI
- [ImNodeFlow](https://github.com/Fattorino/ImNodeFlow) - Node graph framework

## License

MIT License
