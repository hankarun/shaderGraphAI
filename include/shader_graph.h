#ifndef SHADER_GRAPH_H
#define SHADER_GRAPH_H

#include "ImNodeFlow.h"
#include "shader_nodes.h"
#include <string>
#include <sstream>
#include <memory>

namespace ShaderGraph {

class ShaderGraphEditor {
public:
    ShaderGraphEditor() : m_nodeFlow("Shader Graph") {
        m_nodeFlow.setSize(ImVec2(0, 0)); // Auto-fit
        
        // Set up right-click popup for adding nodes
        m_nodeFlow.rightClickPopUpContent([this](ImFlow::BaseNode* node) {
            if (node) {
                // Right-clicked on a node
                if (ImGui::MenuItem("Delete Node")) {
                    node->destroy();
                }
            } else {
                // Right-clicked on empty space - show add node menu
                showAddNodeMenu();
            }
        });
        
        // Create default output node
        m_outputNode = m_nodeFlow.addNode<OutputNode>(ImVec2(600, 200));
        
        // Create a simple default setup
        auto colorNode = m_nodeFlow.addNode<ColorNode>(ImVec2(100, 150));
        colorNode->outPin("RGB")->createLink(m_outputNode->inPin("Color"));
    }
    
    void update() {
        m_nodeFlow.update();
    }
    
    void setSize(const ImVec2& size) {
        m_nodeFlow.setSize(size);
    }
    
    // Generate fragment shader code from the node graph
    std::string generateFragmentShader() {
        std::stringstream ss;
        
        // Shader header
        ss << R"(#version 330 core
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
)";
        
        // Generate the main shader code from output node
        if (m_outputNode) {
            ss << m_outputNode->generateCode();
        } else {
            ss << "    FragColor = vec4(1.0, 0.0, 1.0, 1.0); // Error: No output node\n";
        }
        
        ss << "}\n";
        
        return ss.str();
    }
    
    ImFlow::ImNodeFlow& getNodeFlow() { return m_nodeFlow; }
    
private:
    void showAddNodeMenu() {
        if (ImGui::BeginMenu("Constants")) {
            if (ImGui::MenuItem("Float")) {
                m_nodeFlow.placeNode<FloatNode>();
            }
            if (ImGui::MenuItem("Color (Vec3)")) {
                m_nodeFlow.placeNode<ColorNode>();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Input")) {
            if (ImGui::MenuItem("Time")) {
                m_nodeFlow.placeNode<TimeNode>();
            }
            if (ImGui::MenuItem("Position")) {
                m_nodeFlow.placeNode<UVNode>();
            }
            if (ImGui::MenuItem("Normal")) {
                m_nodeFlow.placeNode<NormalNode>();
            }
            if (ImGui::MenuItem("Fresnel")) {
                m_nodeFlow.placeNode<FresnelNode>();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Math")) {
            if (ImGui::MenuItem("Add")) {
                m_nodeFlow.placeNode<AddNode>();
            }
            if (ImGui::MenuItem("Subtract")) {
                m_nodeFlow.placeNode<SubtractNode>();
            }
            if (ImGui::MenuItem("Multiply")) {
                m_nodeFlow.placeNode<MultiplyNode>();
            }
            if (ImGui::MenuItem("Divide")) {
                m_nodeFlow.placeNode<DivideNode>();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Sin")) {
                m_nodeFlow.placeNode<SinNode>();
            }
            if (ImGui::MenuItem("Cos")) {
                m_nodeFlow.placeNode<CosNode>();
            }
            if (ImGui::MenuItem("Abs")) {
                m_nodeFlow.placeNode<AbsNode>();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Mix (Lerp)")) {
                m_nodeFlow.placeNode<MixNode>();
            }
            if (ImGui::MenuItem("Clamp")) {
                m_nodeFlow.placeNode<ClampNode>();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Vector")) {
            if (ImGui::MenuItem("Make Vec3")) {
                m_nodeFlow.placeNode<MakeVec3Node>();
            }
            if (ImGui::MenuItem("Split Vec3")) {
                m_nodeFlow.placeNode<SplitVec3Node>();
            }
            ImGui::EndMenu();
        }
    }
    
    ImFlow::ImNodeFlow m_nodeFlow;
    std::shared_ptr<OutputNode> m_outputNode;
};

} // namespace ShaderGraph

#endif // SHADER_GRAPH_H
