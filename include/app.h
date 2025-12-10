#ifndef APP_H
#define APP_H

#include <string>
#include <memory>

struct GLFWwindow;

namespace ShaderGraph {
    class ShaderGraphEditor;
}

class App {
public:
    App();
    ~App();
    
    void init(int width, int height, const std::string& name);
    void run();

private:
    void initWindow();
    void initImGui();
    void initCubeRenderer();
    void initFramebuffer(int width, int height);
    void initShaderGraph();
    void compileShaders();
    void shutdown();
    void render();
    void renderCubeToTexture();
    void renderPreviewWindow();
    void renderShaderEditorWindow();
    void renderNodeGraphWindow();
    void updateShaderFromGraph();

    GLFWwindow* m_window = nullptr;
    int m_windowWidth = 1280;
    int m_windowHeight = 720;
    std::string m_windowName = "ShaderGraph";
    
    // Framebuffer for rendering cube to texture
    unsigned int m_framebuffer = 0;
    unsigned int m_textureColorbuffer = 0;
    unsigned int m_rbo = 0;
    int m_fbWidth = 512;
    int m_fbHeight = 512;
    
    // Cube rendering
    unsigned int m_cubeVAO = 0;
    unsigned int m_cubeVBO = 0;
    unsigned int m_shaderProgram = 0;
    
    // Shader source code (generated from graph - read only in editor)
    std::string m_vertexShaderSource;
    std::string m_fragmentShaderSource;
    
    // Shader compilation status
    bool m_shaderCompileError = false;
    std::string m_shaderErrorLog;
    
    // Animation
    float m_rotationAngle = 0.0f;
    float m_time = 0.0f;
    
    // Node Graph Editor
    std::unique_ptr<ShaderGraph::ShaderGraphEditor> m_shaderGraph;
    bool m_autoCompile = true;
    std::string m_lastGeneratedCode;
};

#endif // APP_H
