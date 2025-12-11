#include "app.h"
#include "mat.h"
#include "shader_graph.h"
#include <iostream>
#include <cstring>
#include <fstream>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#elif defined(_WIN32)
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// Default vertex shader
static const char* defaultVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// Default fragment shader
static const char* defaultFragmentShader = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform float time;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor;
    
    // Color variation with time
    vec3 color = objectColor * (0.8 + 0.2 * sin(time));
    
    vec3 result = (ambient + diffuse + specular) * color;
    FragColor = vec4(result, 1.0);
}
)";

// Cube vertices with normals
static float cubeVertices[] = {
    // positions          // normals
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

App::App() {
    std::cout << "App created" << std::endl;
}

void App::init(int width, int height, const std::string& name) {
    m_windowWidth = width;
    m_windowHeight = height;
    m_windowName = name;
    
    initWindow();
    initImGui();
    initCubeRenderer();
    initFramebuffer(m_fbWidth, m_fbHeight);
    initShaderGraph();
    
    // Initialize shader sources
    m_vertexShaderSource = defaultVertexShader;
    
    // Generate initial fragment shader from graph
    updateShaderFromGraph();
    
    std::cout << "App initialized" << std::endl;
}

App::~App() {
    shutdown();
    std::cout << "App destroyed" << std::endl;
}

void App::initWindow() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    // Set OpenGL version (3.3 core for macOS compatibility)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, m_windowName.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // Enable vsync

#ifdef _WIN32
    // Initialize GLEW for Windows OpenGL extension loading
    glewExperimental = GL_TRUE;
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(glewErr) << std::endl;
        glfwDestroyWindow(m_window);
        glfwTerminate();
        m_window = nullptr;
        return;
    }
    std::cout << "GLEW initialized successfully" << std::endl;
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
#endif
}

void App::initImGui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    // Set ini file path for saving window layout
    io.IniFilename = "imgui.ini";
    
    // Check if imgui.ini exists - if not, we'll reset layout on first frame
    std::ifstream iniFile("imgui.ini");
    m_resetLayout = !iniFile.good();
    iniFile.close();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void App::initCubeRenderer() {
    // Create VAO and VBO for the cube
    glGenVertexArrays(1, &m_cubeVAO);
    glGenBuffers(1, &m_cubeVBO);
    
    glBindVertexArray(m_cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

void App::initFramebuffer(int width, int height) {
    // Create framebuffer
    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    
    // Create texture to render to
    glGenTextures(1, &m_textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, m_textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureColorbuffer, 0);
    
    // Create renderbuffer for depth and stencil
    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void App::compileShaders() {
    m_shaderCompileError = false;
    m_shaderErrorLog.clear();
    
    // Delete old shader program if exists
    if (m_shaderProgram != 0) {
        glDeleteProgram(m_shaderProgram);
    }
    
    // Compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertSrc = m_vertexShaderSource.c_str();
    glShaderSource(vertexShader, 1, &vertSrc, NULL);
    glCompileShader(vertexShader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        m_shaderCompileError = true;
        m_shaderErrorLog = "VERTEX SHADER ERROR:\n" + std::string(infoLog);
        glDeleteShader(vertexShader);
        return;
    }
    
    // Compile fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragSrc = m_fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &fragSrc, NULL);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        m_shaderCompileError = true;
        m_shaderErrorLog = "FRAGMENT SHADER ERROR:\n" + std::string(infoLog);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return;
    }
    
    // Link shaders
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);
    
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_shaderProgram, 512, NULL, infoLog);
        m_shaderCompileError = true;
        m_shaderErrorLog = "SHADER LINKING ERROR:\n" + std::string(infoLog);
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void App::initShaderGraph() {
    m_shaderGraph = std::make_unique<ShaderGraph::ShaderGraphEditor>();
}

void App::updateShaderFromGraph() {
    if (!m_shaderGraph) return;
    
    std::string newCode = m_shaderGraph->generateFragmentShader();
    
    // Only recompile if code changed
    if (newCode != m_lastGeneratedCode) {
        m_lastGeneratedCode = newCode;
        m_fragmentShaderSource = newCode;
        compileShaders();
    }
}

void App::shutdown() {
    // Cleanup OpenGL resources
    if (m_cubeVAO) glDeleteVertexArrays(1, &m_cubeVAO);
    if (m_cubeVBO) glDeleteBuffers(1, &m_cubeVBO);
    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
    if (m_framebuffer) glDeleteFramebuffers(1, &m_framebuffer);
    if (m_textureColorbuffer) glDeleteTextures(1, &m_textureColorbuffer);
    if (m_rbo) glDeleteRenderbuffers(1, &m_rbo);
    
    // Save ImGui window layout before shutdown
    ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

void App::renderCubeToTexture() {
    // Bind our framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glViewport(0, 0, m_fbWidth, m_fbHeight);
    
    // Clear the framebuffer
    glClearColor(0.15f, 0.15f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    
    if (m_shaderProgram && !m_shaderCompileError) {
        glUseProgram(m_shaderProgram);
        
        // Create transformation matrices
        float model[16], view[16], projection[16];
        float rotY[16], rotX[16];
        
        // Model matrix - rotate the cube
        mat::rotateY(rotY, m_rotationAngle);
        mat::rotateX(rotX, m_rotationAngle * 0.5f);
        mat::multiply(model, rotY, rotX);
        
        // View matrix - camera position
        mat::lookAt(view, 0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        
        // Projection matrix
        mat::perspective(projection, 45.0f * 3.14159f / 180.0f, (float)m_fbWidth / (float)m_fbHeight, 0.1f, 100.0f);
        
        // Set uniforms
        glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "model"), 1, GL_FALSE, model);
        glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "view"), 1, GL_FALSE, view);
        glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "projection"), 1, GL_FALSE, projection);
        glUniform1f(glGetUniformLocation(m_shaderProgram, "time"), m_time);
        glUniform3f(glGetUniformLocation(m_shaderProgram, "lightPos"), 2.0f, 2.0f, 2.0f);
        glUniform3f(glGetUniformLocation(m_shaderProgram, "viewPos"), 0.0f, 0.0f, 3.0f);
        glUniform3f(glGetUniformLocation(m_shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(m_shaderProgram, "objectColor"), 0.3f, 0.6f, 0.9f);
        
        // Draw the cube
        glBindVertexArray(m_cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
    
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void App::renderPreviewWindow() {
    ImGui::Begin("Shader Preview");
    
    // Get available size for the image
    ImVec2 availSize = ImGui::GetContentRegionAvail();
    float aspect = (float)m_fbWidth / (float)m_fbHeight;
    
    ImVec2 imageSize;
    if (availSize.x / aspect <= availSize.y) {
        imageSize.x = availSize.x;
        imageSize.y = availSize.x / aspect;
    } else {
        imageSize.y = availSize.y;
        imageSize.x = availSize.y * aspect;
    }
    
    // Display the rendered texture (flip Y by swapping UV coordinates)
    ImGui::Image((ImTextureID)(intptr_t)m_textureColorbuffer, imageSize, ImVec2(0, 1), ImVec2(1, 0));
    
    ImGui::End();
}

void App::renderShaderEditorWindow() {
    ImGui::Begin("Generated Shader (Read-Only)");
    
    // Display compilation status at the top
    if (m_shaderCompileError) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::TextWrapped("Compilation Error: %s", m_shaderErrorLog.c_str());
        ImGui::PopStyleColor();
        ImGui::Separator();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
        ImGui::Text("Shader compiled successfully!");
        ImGui::PopStyleColor();
        ImGui::Separator();
    }
    
    // Auto-compile toggle
    ImGui::Checkbox("Auto-compile on graph change", &m_autoCompile);
    ImGui::SameLine();
    if (ImGui::Button("Compile Now")) {
        updateShaderFromGraph();
    }
    
    ImGui::Separator();
    
    // Tabs for vertex and fragment shaders (read-only)
    if (ImGui::BeginTabBar("ShaderTabs")) {
        if (ImGui::BeginTabItem("Fragment Shader")) {
            // Read-only text display
            ImGui::BeginChild("FragmentShaderCode", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
            ImGui::TextUnformatted(m_fragmentShaderSource.c_str());
            ImGui::PopStyleColor();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Vertex Shader")) {
            // Read-only text display
            ImGui::BeginChild("VertexShaderCode", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
            ImGui::TextUnformatted(m_vertexShaderSource.c_str());
            ImGui::PopStyleColor();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    
    ImGui::End();
}

void App::renderNodeGraphWindow() {
    ImGui::Begin("Node Graph");
    
    ImGui::Text("Right-click to add nodes. Connect outputs to inputs.");
    ImGui::Separator();
    
    // Get available size for the node graph
    ImVec2 availSize = ImGui::GetContentRegionAvail();
    
    if (m_shaderGraph) {
        m_shaderGraph->setSize(availSize);
        m_shaderGraph->update();
        
        // Auto-compile when graph changes
        if (m_autoCompile) {
            updateShaderFromGraph();
        }
    }
    
    ImGui::End();
}

void App::render() {
    // Update animation
    m_time = (float)glfwGetTime();
    m_rotationAngle += 0.01f;
    
    // Render cube to texture
    renderCubeToTexture();
    
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Enable dockspace
    ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    
    // Reset layout on first frame if no imgui.ini was found
    if (m_resetLayout) {
        m_resetLayout = false;
        
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);
        
        // Split the dockspace: left for preview, right for shader editor and node graph
        ImGuiID dock_left, dock_right;
        ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.3f, &dock_left, &dock_right);
        
        // Split the right side: top for shader editor, bottom for node graph
        ImGuiID dock_right_top, dock_right_bottom;
        ImGui::DockBuilderSplitNode(dock_right, ImGuiDir_Up, 0.5f, &dock_right_top, &dock_right_bottom);
        
        // Dock windows
        ImGui::DockBuilderDockWindow("Shader Preview", dock_left);
        ImGui::DockBuilderDockWindow("Generated Shader (Read-Only)", dock_right_top);
        ImGui::DockBuilderDockWindow("Node Graph", dock_right_bottom);
        
        ImGui::DockBuilderFinish(dockspace_id);
    }

    // Render ImGui windows
    renderPreviewWindow();
    renderNodeGraphWindow();
    renderShaderEditorWindow();
    
    // Stats window
    ImGui::Begin("ShaderGraph");
    ImGui::Text("Welcome to ShaderGraph!");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::Text("Time: %.2f", m_time);
    ImGui::Text("Rotation: %.2f", m_rotationAngle);
    ImGui::Separator();
    ImGui::TextWrapped("Instructions:");
    ImGui::BulletText("Right-click in Node Graph to add nodes");
    ImGui::BulletText("Drag from output to input pins to connect");
    ImGui::BulletText("The shader code is auto-generated");
    ImGui::End();

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(m_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window);
}

void App::run() {
    std::cout << "App is running..." << std::endl;
    
    while (m_window && !glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        render();
    }
}
