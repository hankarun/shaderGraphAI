#ifndef APP_H
#define APP_H

#include <string>

struct GLFWwindow;

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
    void compileShaders();
    void shutdown();
    void render();
    void renderCubeToTexture();
    void renderPreviewWindow();
    void renderShaderEditorWindow();

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
    
    // Shader source code (editable)
    std::string m_vertexShaderSource;
    std::string m_fragmentShaderSource;
    char m_vertexShaderBuffer[8192];
    char m_fragmentShaderBuffer[8192];
    
    // Shader compilation status
    bool m_shaderCompileError = false;
    std::string m_shaderErrorLog;
    
    // Animation
    float m_rotationAngle = 0.0f;
    float m_time = 0.0f;
};

#endif // APP_H
