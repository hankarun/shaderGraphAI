#ifndef SHADER_GRAPH_H
#define SHADER_GRAPH_H

#include "ImNodeFlow.h"
#include "shader_nodes.h"
#include <string>
#include <sstream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <algorithm>

namespace ShaderGraph {

// Graph traversal helper class
class GraphTraverser {
public:
    // Topologically sort nodes starting from the output node
    // Returns nodes in order of dependency (sources first, output last)
    static std::vector<ShaderNodeBase*> topologicalSort(OutputNode* outputNode, ImFlow::ImNodeFlow& nodeFlow) {
        std::vector<ShaderNodeBase*> result;
        std::unordered_set<ImFlow::NodeUID> visited;
        std::unordered_set<ImFlow::NodeUID> inStack;
        
        // Collect all connected nodes via DFS
        std::vector<ShaderNodeBase*> connectedNodes;
        collectConnectedNodes(outputNode, connectedNodes, visited);
        
        // Reset visited for topological sort
        visited.clear();
        
        // Perform topological sort using DFS
        for (auto* node : connectedNodes) {
            if (visited.find(node->getUID()) == visited.end()) {
                topologicalSortDFS(node, visited, inStack, result);
            }
        }
        
        return result;
    }
    
private:
    // DFS to collect all nodes connected to the output
    static void collectConnectedNodes(ImFlow::BaseNode* node, 
                                     std::vector<ShaderNodeBase*>& nodes,
                                     std::unordered_set<ImFlow::NodeUID>& visited) {
        if (!node || visited.find(node->getUID()) != visited.end()) return;
        visited.insert(node->getUID());
        
        ShaderNodeBase* shaderNode = dynamic_cast<ShaderNodeBase*>(node);
        if (shaderNode) {
            nodes.push_back(shaderNode);
        }
        
        // Visit all input connections
        for (auto& pin : node->getIns()) {
            if (pin && pin->isConnected()) {
                auto linkWeak = pin->getLink();
                if (auto link = linkWeak.lock()) {
                    auto* leftPin = link->left();
                    if (leftPin && leftPin->getParent()) {
                        collectConnectedNodes(leftPin->getParent(), nodes, visited);
                    }
                }
            }
        }
    }
    
    // DFS for topological sorting
    static void topologicalSortDFS(ShaderNodeBase* node,
                                   std::unordered_set<ImFlow::NodeUID>& visited,
                                   std::unordered_set<ImFlow::NodeUID>& inStack,
                                   std::vector<ShaderNodeBase*>& result) {
        if (!node) return;
        
        ImFlow::NodeUID nodeId = node->getUID();
        if (inStack.find(nodeId) != inStack.end()) {
            // Cycle detected - skip
            return;
        }
        if (visited.find(nodeId) != visited.end()) {
            return;
        }
        
        inStack.insert(nodeId);
        
        // Visit all nodes this one depends on (input connections)
        for (auto& pin : node->getIns()) {
            if (pin && pin->isConnected()) {
                auto linkWeak = pin->getLink();
                if (auto link = linkWeak.lock()) {
                    auto* leftPin = link->left();
                    if (leftPin && leftPin->getParent()) {
                        ShaderNodeBase* parentNode = dynamic_cast<ShaderNodeBase*>(leftPin->getParent());
                        if (parentNode) {
                            topologicalSortDFS(parentNode, visited, inStack, result);
                        }
                    }
                }
            }
        }
        
        inStack.erase(nodeId);
        visited.insert(nodeId);
        result.push_back(node);
    }
};

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
        // Collect parameters from graph
        collectParameters();
    }
    
    void setSize(const ImVec2& size) {
        m_nodeFlow.setSize(size);
    }
    
    // Get all parameters in the graph
    const std::vector<UniformParameter>& getParameters() const { return m_parameters; }
    
    // Update parameter value in a node
    void setParameterValue(const std::string& uniformName, const UniformParameter& param) {
        for (auto& nodePair : m_nodeFlow.getNodes()) {
            ShaderNodeBase* shaderNode = dynamic_cast<ShaderNodeBase*>(nodePair.second.get());
            if (shaderNode && shaderNode->isParameterNode()) {
                UniformParameter nodeParam = shaderNode->getUniformParameter();
                if (nodeParam.name == uniformName) {
                    shaderNode->setUniformValue(param);
                    break;
                }
            }
        }
    }
    
    // Generate fragment shader code using graph traversal
    std::string generateFragmentShader() {
        std::stringstream ss;
        
        // Shader header
        ss << R"(#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// Built-in uniforms
uniform float time;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

)";
        
        // Generate user parameter uniforms
        for (const auto& param : m_parameters) {
            ss << "// User parameter: " << param.displayName << "\n";
            ss << "uniform " << ShaderNodeBase::typeToGLSL(param.type) << " " << param.name << ";\n";
        }
        
        ss << R"(
void main()
{
)";
        
        // Generate the main shader code using graph traversal
        if (m_outputNode) {
            ss << generateShaderBody();
        } else {
            ss << "    FragColor = vec4(1.0, 0.0, 1.0, 1.0); // Error: No output node\n";
        }
        
        ss << "}\n";
        
        return ss.str();
    }
    
    // Generate shader body using topological traversal
    std::string generateShaderBody() {
        std::stringstream ss;
        
        // Get topologically sorted nodes
        auto sortedNodes = GraphTraverser::topologicalSort(m_outputNode.get(), m_nodeFlow);
        
        // Map: nodeUID -> (pinName -> varName/expression)
        std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>> varMap;
        
        // Track which nodes have multiple outgoing connections (need variables)
        std::unordered_map<ImFlow::NodeUID, int> outConnectionCount;
        countOutputConnections(sortedNodes, outConnectionCount);
        
        int varCounter = 0;
        
        // Process nodes in topological order
        for (auto* node : sortedNodes) {
            // Skip the output node - handle it specially at the end
            if (node == m_outputNode.get()) continue;
            
            ImFlow::NodeUID nodeId = node->getUID();
            
            // Check if this node needs a variable (has multiple outputs or complex expression)
            bool needsVariable = outConnectionCount[nodeId] > 1 || !node->isSourceNode();
            (void)needsVariable; // Suppress unused warning
            
            // Get all output pins
            for (auto& pin : node->getOuts()) {
                std::string pinName = pin->getName();
                std::string expr = node->generateExpression(pinName, varMap);
                
                // Source nodes with single output use inline expression
                if (node->isSourceNode() && outConnectionCount[nodeId] <= 1) {
                    varMap[nodeId][pinName] = expr;
                } else {
                    // Generate a variable for this output
                    std::string varName = "v" + std::to_string(varCounter++);
                    ShaderDataType type = node->getOutputType(pinName);
                    std::string typeStr = ShaderNodeBase::typeToGLSL(type);
                    
                    ss << "    " << typeStr << " " << varName << " = " << expr << ";\n";
                    varMap[nodeId][pinName] = varName;
                }
            }
        }
        
        // Add spacing before final output if we generated variables
        if (varCounter > 0) {
            ss << "\n";
        }
        
        // Generate final output
        ss << m_outputNode->generateCodeFromVarMap(varMap);
        
        return ss.str();
    }
    
    ImFlow::ImNodeFlow& getNodeFlow() { return m_nodeFlow; }
    
private:
    // Count how many connections each node's output has
    void countOutputConnections(const std::vector<ShaderNodeBase*>& nodes,
                                std::unordered_map<ImFlow::NodeUID, int>& outCount) {
        for (auto* node : nodes) {
            outCount[node->getUID()] = 0;
        }
        
        // Count incoming connections (which represent output usages)
        for (auto* node : nodes) {
            for (auto& pin : node->getIns()) {
                if (pin && pin->isConnected()) {
                    auto linkWeak = pin->getLink();
                    if (auto link = linkWeak.lock()) {
                        auto* leftPin = link->left();
                        if (leftPin && leftPin->getParent()) {
                            ImFlow::NodeUID srcId = leftPin->getParent()->getUID();
                            outCount[srcId]++;
                        }
                    }
                }
            }
        }
    }
    
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
        
        if (ImGui::BeginMenu("Parameters")) {
            if (ImGui::MenuItem("Float Parameter")) {
                m_nodeFlow.placeNode<FloatParameterNode>();
            }
            if (ImGui::MenuItem("Vec3 Parameter (Color)")) {
                m_nodeFlow.placeNode<Vec3ParameterNode>();
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
            if (ImGui::MenuItem("Tex Coord (UV)")) {
                m_nodeFlow.placeNode<TexCoordNode>();
            }
            if (ImGui::MenuItem("Fresnel")) {
                m_nodeFlow.placeNode<FresnelNode>();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Texture")) {
            if (ImGui::MenuItem("Texture Sampler")) {
                m_nodeFlow.placeNode<TextureNode>();
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
    std::vector<UniformParameter> m_parameters;  // Collected user parameters
    
    // Collect all parameter nodes from the graph
    void collectParameters() {
        m_parameters.clear();
        for (auto& nodePair : m_nodeFlow.getNodes()) {
            ShaderNodeBase* shaderNode = dynamic_cast<ShaderNodeBase*>(nodePair.second.get());
            if (shaderNode && shaderNode->isParameterNode()) {
                m_parameters.push_back(shaderNode->getUniformParameter());
            }
        }
    }
};

} // namespace ShaderGraph

#endif // SHADER_GRAPH_H
