#ifndef SHADER_NODES_H
#define SHADER_NODES_H

#include "ImNodeFlow.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>

namespace ShaderGraph {

// Forward declarations
class ShaderNode;

// Custom node styles with proper padding
inline std::shared_ptr<ImFlow::NodeStyle> InputNodeStyle() {
    auto style = std::make_shared<ImFlow::NodeStyle>(IM_COL32(90,191,93,255), ImColor(233,241,244,255), 6.5f);
    style->padding = ImVec4(20.f, 8.f, 20.f, 8.f);
    return style;
}

inline std::shared_ptr<ImFlow::NodeStyle> MathNodeStyle() {
    auto style = std::make_shared<ImFlow::NodeStyle>(IM_COL32(71,142,173,255), ImColor(233,241,244,255), 6.5f);
    style->padding = ImVec4(20.f, 8.f, 20.f, 8.f);
    return style;
}

inline std::shared_ptr<ImFlow::NodeStyle> VectorNodeStyle() {
    auto style = std::make_shared<ImFlow::NodeStyle>(IM_COL32(191,134,90,255), ImColor(233,241,244,255), 6.5f);
    style->padding = ImVec4(20.f, 8.f, 20.f, 8.f);
    return style;
}

inline std::shared_ptr<ImFlow::NodeStyle> OutputNodeStyle() {
    auto style = std::make_shared<ImFlow::NodeStyle>(IM_COL32(191,90,90,255), ImColor(233,241,244,255), 6.5f);
    style->padding = ImVec4(20.f, 8.f, 20.f, 8.f);
    return style;
}

// Pin styles for different data types
inline std::shared_ptr<ImFlow::PinStyle> FloatPinStyle() {
    return ImFlow::PinStyle::cyan();
}

inline std::shared_ptr<ImFlow::PinStyle> Vec2PinStyle() {
    return ImFlow::PinStyle::green();
}

inline std::shared_ptr<ImFlow::PinStyle> Vec3PinStyle() {
    return ImFlow::PinStyle::blue();
}

inline std::shared_ptr<ImFlow::PinStyle> Vec4PinStyle() {
    return ImFlow::PinStyle::brown();
}

// Shader code result structure
struct ShaderCode {
    std::string code;        // The expression or variable name
    std::string declaration; // Any variable declarations needed
    
    ShaderCode() : code("0.0"), declaration("") {}
    ShaderCode(const std::string& c) : code(c), declaration("") {}
    ShaderCode(const std::string& c, const std::string& d) : code(c), declaration(d) {}
};

// Data type enumeration for shader variables
enum class ShaderDataType {
    Float,
    Vec2,
    Vec3,
    Vec4
};

// Structure to hold generated variable info
struct GeneratedVar {
    std::string varName;
    std::string expression;
    ShaderDataType type;
    bool isGenerated = false;
};

// Base class for shader nodes with code generation support
class ShaderNodeBase : public ImFlow::BaseNode {
public:
    virtual ~ShaderNodeBase() = default;
    
    // Get the unique ID for this node (used for variable naming)
    ImFlow::NodeUID getNodeUID() const { return getUID(); }
    
    // Check if this is an input/constant node (no inputs)
    virtual bool isSourceNode() const { return false; }
    
    // Get the GLSL type string for a data type
    static std::string typeToGLSL(ShaderDataType type) {
        switch (type) {
            case ShaderDataType::Float: return "float";
            case ShaderDataType::Vec2: return "vec2";
            case ShaderDataType::Vec3: return "vec3";
            case ShaderDataType::Vec4: return "vec4";
        }
        return "float";
    }
    
    // Generate unique variable name for this node's output
    std::string getOutputVarName(const std::string& pinName = "") const {
        std::string baseName = "node_" + std::to_string(getUID());
        if (!pinName.empty()) {
            baseName += "_" + pinName;
        }
        return baseName;
    }
    
    // Get the output data type for a specific pin
    virtual ShaderDataType getOutputType(const std::string& pinName) const {
        return ShaderDataType::Float;
    }
    
    // Generate the expression for an output pin (used during traversal)
    virtual std::string generateExpression(const std::string& pinName,
                                           const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap) const {
        return "0.0";
    }
};

// ============================================================================
// FLOAT CONSTANT NODE - Outputs a constant float value
// ============================================================================
class FloatNode : public ShaderNodeBase {
public:
    FloatNode() {
        setTitle("Float");
        setStyle(MathNodeStyle());
        addOUT<ShaderCode>("Value", FloatPinStyle())->behaviour([this]() {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(3) << m_value;
            return ShaderCode(ss.str());
        });
    }

    void draw() override {
        ImGui::SetNextItemWidth(80.f);
        ImGui::DragFloat("##value", &m_value, 0.01f, -100.0f, 100.0f, "%.3f");
    }
    
    bool isSourceNode() const override { return true; }
    ShaderDataType getOutputType(const std::string& pinName) const override { return ShaderDataType::Float; }
    
    std::string generateExpression(const std::string& pinName,
                                   const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap) const override {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(3) << m_value;
        return ss.str();
    }
    
    float getValue() const { return m_value; }

private:
    float m_value = 0.0f;
};

// ============================================================================
// VEC3 COLOR NODE - Outputs a vec3 color value
// ============================================================================
class ColorNode : public ShaderNodeBase {
public:
    ColorNode() {
        setTitle("Color");
        setStyle(VectorNodeStyle());
        addOUT<ShaderCode>("RGB", Vec3PinStyle())->behaviour([this]() {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(3);
            ss << "vec3(" << m_color[0] << ", " << m_color[1] << ", " << m_color[2] << ")";
            return ShaderCode(ss.str());
        });
    }

    void draw() override {
        ImGui::SetNextItemWidth(150.f);
        ImGui::ColorEdit3("##color", m_color, ImGuiColorEditFlags_NoInputs);
    }
    
    bool isSourceNode() const override { return true; }
    ShaderDataType getOutputType(const std::string& pinName) const override { return ShaderDataType::Vec3; }
    
    std::string generateExpression(const std::string& pinName,
                                   const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap) const override {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(3);
        ss << "vec3(" << m_color[0] << ", " << m_color[1] << ", " << m_color[2] << ")";
        return ss.str();
    }

private:
    float m_color[3] = {1.0f, 0.5f, 0.2f};
};

// ============================================================================
// TIME NODE - Outputs the time uniform
// ============================================================================
class TimeNode : public ShaderNodeBase {
public:
    TimeNode() {
        setTitle("Time");
        setStyle(InputNodeStyle());
        addOUT<ShaderCode>("Time", FloatPinStyle())->behaviour([this]() {
            return ShaderCode("time");
        });
    }

    void draw() override {
        ImGui::Text("Uniform: time");
    }
    
    bool isSourceNode() const override { return true; }
    ShaderDataType getOutputType(const std::string& pinName) const override { return ShaderDataType::Float; }
    
    std::string generateExpression(const std::string& pinName,
                                   const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap) const override {
        return "time";
    }
};

// ============================================================================
// UV NODE - Outputs UV coordinates (FragPos based)
// ============================================================================
class UVNode : public ShaderNodeBase {
public:
    UVNode() {
        setTitle("Position");
        setStyle(InputNodeStyle());
        addOUT<ShaderCode>("XYZ", Vec3PinStyle())->behaviour([this]() {
            return ShaderCode("FragPos");
        });
        addOUT<ShaderCode>("X", FloatPinStyle())->behaviour([this]() {
            return ShaderCode("FragPos.x");
        });
        addOUT<ShaderCode>("Y", FloatPinStyle())->behaviour([this]() {
            return ShaderCode("FragPos.y");
        });
        addOUT<ShaderCode>("Z", FloatPinStyle())->behaviour([this]() {
            return ShaderCode("FragPos.z");
        });
    }

    void draw() override {
        ImGui::Text("Fragment Pos");
    }
    
    bool isSourceNode() const override { return true; }
    
    ShaderDataType getOutputType(const std::string& pinName) const override {
        if (pinName == "XYZ") return ShaderDataType::Vec3;
        return ShaderDataType::Float;
    }
    
    std::string generateExpression(const std::string& pinName,
                                   const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap) const override {
        if (pinName == "XYZ") return "FragPos";
        if (pinName == "X") return "FragPos.x";
        if (pinName == "Y") return "FragPos.y";
        if (pinName == "Z") return "FragPos.z";
        return "FragPos";
    }
};

// ============================================================================
// NORMAL NODE - Outputs the normal vector
// ============================================================================
class NormalNode : public ShaderNodeBase {
public:
    NormalNode() {
        setTitle("Normal");
        setStyle(InputNodeStyle());
        addOUT<ShaderCode>("Normal", Vec3PinStyle())->behaviour([this]() {
            return ShaderCode("normalize(Normal)");
        });
    }

    void draw() override {
        ImGui::Text("Surface Normal");
    }
    
    bool isSourceNode() const override { return true; }
    ShaderDataType getOutputType(const std::string& pinName) const override { return ShaderDataType::Vec3; }
    
    std::string generateExpression(const std::string& pinName,
                                   const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap) const override {
        return "normalize(Normal)";
    }
};

// Helper function to get input variable from the varMap or default
inline std::string getInputVar(const ImFlow::BaseNode* node, const std::string& pinName,
                               const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap,
                               const std::string& defaultVal) {
    // Need to cast away const since ImNodeFlow doesn't have const methods
    auto* mutableNode = const_cast<ImFlow::BaseNode*>(node);
    auto* pin = mutableNode->inPin(pinName);
    if (pin && pin->isConnected()) {
        auto linkWeak = pin->getLink();
        if (auto link = linkWeak.lock()) {
            auto* leftPin = link->left();
            if (leftPin && leftPin->getParent()) {
                ImFlow::NodeUID srcNodeId = leftPin->getParent()->getUID();
                std::string srcPinName = leftPin->getName();
                auto nodeIt = varMap.find(srcNodeId);
                if (nodeIt != varMap.end()) {
                    auto pinIt = nodeIt->second.find(srcPinName);
                    if (pinIt != nodeIt->second.end()) {
                        return pinIt->second;
                    }
                }
            }
        }
    }
    return defaultVal;
}

// ============================================================================
// ADD NODE - Adds two values
// ============================================================================
class AddNode : public ShaderNodeBase {
public:
    AddNode() {
        setTitle("Add");
        setStyle(MathNodeStyle());
        addIN<ShaderCode>("A", ShaderCode("0.0"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
        addIN<ShaderCode>("B", ShaderCode("0.0"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
        addOUT<ShaderCode>("Result", FloatPinStyle())->behaviour([this]() {
            auto a = getInVal<ShaderCode>("A");
            auto b = getInVal<ShaderCode>("B");
            return ShaderCode("(" + a.code + " + " + b.code + ")");
        });
    }

    void draw() override {
        ImGui::Text("A + B");
    }
    
    ShaderDataType getOutputType(const std::string& pinName) const override { return ShaderDataType::Float; }
    
    std::string generateExpression(const std::string& pinName,
                                   const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap) const override {
        std::string a = getInputVar(this, "A", varMap, "0.0");
        std::string b = getInputVar(this, "B", varMap, "0.0");
        return "(" + a + " + " + b + ")";
    }
};

// ============================================================================
// MULTIPLY NODE - Multiplies two values (supports float * float, vec3 * vec3, float * vec3)
// ============================================================================
class MultiplyNode : public ShaderNodeBase {
public:
    MultiplyNode() {
        setTitle("Multiply");
        setStyle(MathNodeStyle());
        addIN<ShaderCode>("A", ShaderCode("1.0"), ImFlow::ConnectionFilter::None(), FloatPinStyle());
        addIN<ShaderCode>("B", ShaderCode("1.0"), ImFlow::ConnectionFilter::None(), FloatPinStyle());
        addOUT<ShaderCode>("Result", FloatPinStyle())->behaviour([this]() {
            auto a = getInVal<ShaderCode>("A");
            auto b = getInVal<ShaderCode>("B");
            return ShaderCode("(" + a.code + " * " + b.code + ")");
        });
    }

    void draw() override {
        ImGui::Text("A * B");
    }
    
    // Determine output type based on input connections
    ShaderDataType getOutputType(const std::string& pinName) const override {
        // Check if either input is connected to a vec3
        auto* mutableThis = const_cast<MultiplyNode*>(this);
        auto* pinA = mutableThis->inPin("A");
        auto* pinB = mutableThis->inPin("B");
        
        bool aIsVec3 = false, bIsVec3 = false;
        
        if (pinA && pinA->isConnected()) {
            auto linkWeak = pinA->getLink();
            if (auto link = linkWeak.lock()) {
                auto* leftPin = link->left();
                if (leftPin && leftPin->getParent()) {
                    ShaderNodeBase* srcNode = dynamic_cast<ShaderNodeBase*>(leftPin->getParent());
                    if (srcNode && srcNode->getOutputType(leftPin->getName()) == ShaderDataType::Vec3) {
                        aIsVec3 = true;
                    }
                }
            }
        }
        
        if (pinB && pinB->isConnected()) {
            auto linkWeak = pinB->getLink();
            if (auto link = linkWeak.lock()) {
                auto* leftPin = link->left();
                if (leftPin && leftPin->getParent()) {
                    ShaderNodeBase* srcNode = dynamic_cast<ShaderNodeBase*>(leftPin->getParent());
                    if (srcNode && srcNode->getOutputType(leftPin->getName()) == ShaderDataType::Vec3) {
                        bIsVec3 = true;
                    }
                }
            }
        }
        
        // If either input is vec3, output is vec3
        if (aIsVec3 || bIsVec3) return ShaderDataType::Vec3;
        return ShaderDataType::Float;
    }
    
    std::string generateExpression(const std::string& pinName,
                                   const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap) const override {
        std::string a = getInputVar(this, "A", varMap, "1.0");
        std::string b = getInputVar(this, "B", varMap, "1.0");
        return "(" + a + " * " + b + ")";
    }
};

// ============================================================================
// SUBTRACT NODE - Subtracts two values
// ============================================================================
class SubtractNode : public ShaderNodeBase {
public:
    SubtractNode() {
        setTitle("Subtract");
        setStyle(MathNodeStyle());
        addIN<ShaderCode>("A", ShaderCode("0.0"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
        addIN<ShaderCode>("B", ShaderCode("0.0"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
        addOUT<ShaderCode>("Result", FloatPinStyle())->behaviour([this]() {
            auto a = getInVal<ShaderCode>("A");
            auto b = getInVal<ShaderCode>("B");
            return ShaderCode("(" + a.code + " - " + b.code + ")");
        });
    }

    void draw() override {
        ImGui::Text("A - B");
    }
    
    ShaderDataType getOutputType(const std::string& pinName) const override { return ShaderDataType::Float; }
    
    std::string generateExpression(const std::string& pinName,
                                   const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap) const override {
        std::string a = getInputVar(this, "A", varMap, "0.0");
        std::string b = getInputVar(this, "B", varMap, "0.0");
        return "(" + a + " - " + b + ")";
    }
};

// ============================================================================
// DIVIDE NODE - Divides two values
// ============================================================================
class DivideNode : public ShaderNodeBase {
public:
    DivideNode() {
        setTitle("Divide");
        setStyle(MathNodeStyle());
        addIN<ShaderCode>("A", ShaderCode("1.0"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
        addIN<ShaderCode>("B", ShaderCode("1.0"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
        addOUT<ShaderCode>("Result", FloatPinStyle())->behaviour([this]() {
            auto a = getInVal<ShaderCode>("A");
            auto b = getInVal<ShaderCode>("B");
            return ShaderCode("(" + a.code + " / " + b.code + ")");
        });
    }

    void draw() override {
        ImGui::Text("A / B");
    }
    
    ShaderDataType getOutputType(const std::string& pinName) const override { return ShaderDataType::Float; }
    
    std::string generateExpression(const std::string& pinName,
                                   const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap) const override {
        std::string a = getInputVar(this, "A", varMap, "1.0");
        std::string b = getInputVar(this, "B", varMap, "1.0");
        return "(" + a + " / " + b + ")";
    }
};

// ============================================================================
// SIN NODE - Sine function
// ============================================================================
class SinNode : public ShaderNodeBase {
public:
    SinNode() {
        setTitle("Sin");
        setStyle(MathNodeStyle());
        addIN<ShaderCode>("X", ShaderCode("0.0"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
        addOUT<ShaderCode>("Result", FloatPinStyle())->behaviour([this]() {
            auto x = getInVal<ShaderCode>("X");
            return ShaderCode("sin(" + x.code + ")");
        });
    }

    void draw() override {
        ImGui::Text("sin(X)");
    }
    
    ShaderDataType getOutputType(const std::string& pinName) const override { return ShaderDataType::Float; }
    
    std::string generateExpression(const std::string& pinName,
                                   const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap) const override {
        std::string x = getInputVar(this, "X", varMap, "0.0");
        return "sin(" + x + ")";
    }
};

// ============================================================================
// COS NODE - Cosine function
// ============================================================================
class CosNode : public ShaderNodeBase {
public:
    CosNode() {
        setTitle("Cos");
        setStyle(MathNodeStyle());
        addIN<ShaderCode>("X", ShaderCode("0.0"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
        addOUT<ShaderCode>("Result", FloatPinStyle())->behaviour([this]() {
            auto x = getInVal<ShaderCode>("X");
            return ShaderCode("cos(" + x.code + ")");
        });
    }

    void draw() override {
        ImGui::Text("cos(X)");
    }
    
    ShaderDataType getOutputType(const std::string& pinName) const override { return ShaderDataType::Float; }
    
    std::string generateExpression(const std::string& pinName,
                                   const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap) const override {
        std::string x = getInputVar(this, "X", varMap, "0.0");
        return "cos(" + x + ")";
    }
};

// ============================================================================
// ABS NODE - Absolute value
// ============================================================================
class AbsNode : public ShaderNodeBase {
public:
    AbsNode() {
        setTitle("Abs");
        setStyle(MathNodeStyle());
        addIN<ShaderCode>("X", ShaderCode("0.0"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
        addOUT<ShaderCode>("Result", FloatPinStyle())->behaviour([this]() {
            auto x = getInVal<ShaderCode>("X");
            return ShaderCode("abs(" + x.code + ")");
        });
    }

    void draw() override {
        ImGui::Text("abs(X)");
    }
    
    ShaderDataType getOutputType(const std::string& pinName) const override { return ShaderDataType::Float; }
    
    std::string generateExpression(const std::string& pinName,
                                   const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap) const override {
        std::string x = getInputVar(this, "X", varMap, "0.0");
        return "abs(" + x + ")";
    }
};

// ============================================================================
// MIX NODE - Linear interpolation
// ============================================================================
class MixNode : public ShaderNodeBase {
public:
    MixNode() {
        setTitle("Mix");
        setStyle(MathNodeStyle());
        addIN<ShaderCode>("A", ShaderCode("0.0"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
        addIN<ShaderCode>("B", ShaderCode("1.0"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
        addIN<ShaderCode>("T", ShaderCode("0.5"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
        addOUT<ShaderCode>("Result", FloatPinStyle())->behaviour([this]() {
            auto a = getInVal<ShaderCode>("A");
            auto b = getInVal<ShaderCode>("B");
            auto t = getInVal<ShaderCode>("T");
            return ShaderCode("mix(" + a.code + ", " + b.code + ", " + t.code + ")");
        });
    }

    void draw() override {
        ImGui::Text("mix(A, B, T)");
    }
    
    ShaderDataType getOutputType(const std::string& pinName) const override { return ShaderDataType::Float; }
    
    std::string generateExpression(const std::string& pinName,
                                   const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap) const override {
        std::string a = getInputVar(this, "A", varMap, "0.0");
        std::string b = getInputVar(this, "B", varMap, "1.0");
        std::string t = getInputVar(this, "T", varMap, "0.5");
        return "mix(" + a + ", " + b + ", " + t + ")";
    }
};

// ============================================================================
// CLAMP NODE - Clamp value
// ============================================================================
class ClampNode : public ShaderNodeBase {
public:
    ClampNode() {
        setTitle("Clamp");
        setStyle(MathNodeStyle());
        addIN<ShaderCode>("X", ShaderCode("0.0"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
        addOUT<ShaderCode>("Result", FloatPinStyle())->behaviour([this]() {
            auto x = getInVal<ShaderCode>("X");
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(3);
            ss << "clamp(" << x.code << ", " << m_min << ", " << m_max << ")";
            return ShaderCode(ss.str());
        });
    }

    void draw() override {
        ImGui::SetNextItemWidth(60.f);
        ImGui::DragFloat("Min", &m_min, 0.01f);
        ImGui::SetNextItemWidth(60.f);
        ImGui::DragFloat("Max", &m_max, 0.01f);
    }
    
    ShaderDataType getOutputType(const std::string& pinName) const override { return ShaderDataType::Float; }
    
    std::string generateExpression(const std::string& pinName,
                                   const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap) const override {
        std::string x = getInputVar(this, "X", varMap, "0.0");
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(3);
        ss << "clamp(" << x << ", " << m_min << ", " << m_max << ")";
        return ss.str();
    }

private:
    float m_min = 0.0f;
    float m_max = 1.0f;
};

// ============================================================================
// MAKE VEC3 NODE - Creates a vec3 from components
// ============================================================================
class MakeVec3Node : public ShaderNodeBase {
public:
    MakeVec3Node() {
        setTitle("Make Vec3");
        setStyle(VectorNodeStyle());
        addIN<ShaderCode>("X", ShaderCode("0.0"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
        addIN<ShaderCode>("Y", ShaderCode("0.0"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
        addIN<ShaderCode>("Z", ShaderCode("0.0"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
        addOUT<ShaderCode>("Vec3", Vec3PinStyle())->behaviour([this]() {
            auto x = getInVal<ShaderCode>("X");
            auto y = getInVal<ShaderCode>("Y");
            auto z = getInVal<ShaderCode>("Z");
            return ShaderCode("vec3(" + x.code + ", " + y.code + ", " + z.code + ")");
        });
    }

    void draw() override {
        ImGui::Text("vec3(X,Y,Z)");
    }
    
    ShaderDataType getOutputType(const std::string& pinName) const override { return ShaderDataType::Vec3; }
    
    std::string generateExpression(const std::string& pinName,
                                   const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap) const override {
        std::string x = getInputVar(this, "X", varMap, "0.0");
        std::string y = getInputVar(this, "Y", varMap, "0.0");
        std::string z = getInputVar(this, "Z", varMap, "0.0");
        return "vec3(" + x + ", " + y + ", " + z + ")";
    }
};

// ============================================================================
// SPLIT VEC3 NODE - Splits a vec3 into components
// ============================================================================
class SplitVec3Node : public ShaderNodeBase {
public:
    SplitVec3Node() {
        setTitle("Split Vec3");
        setStyle(VectorNodeStyle());
        addIN<ShaderCode>("Vec3", ShaderCode("vec3(0.0)"), ImFlow::ConnectionFilter::SameType(), Vec3PinStyle());
        addOUT<ShaderCode>("X", FloatPinStyle())->behaviour([this]() {
            auto v = getInVal<ShaderCode>("Vec3");
            return ShaderCode("(" + v.code + ").x");
        });
        addOUT<ShaderCode>("Y", FloatPinStyle())->behaviour([this]() {
            auto v = getInVal<ShaderCode>("Vec3");
            return ShaderCode("(" + v.code + ").y");
        });
        addOUT<ShaderCode>("Z", FloatPinStyle())->behaviour([this]() {
            auto v = getInVal<ShaderCode>("Vec3");
            return ShaderCode("(" + v.code + ").z");
        });
    }

    void draw() override {
        ImGui::Text("Split XYZ");
    }
    
    ShaderDataType getOutputType(const std::string& pinName) const override { return ShaderDataType::Float; }
    
    std::string generateExpression(const std::string& pinName,
                                   const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap) const override {
        std::string v = getInputVar(this, "Vec3", varMap, "vec3(0.0)");
        if (pinName == "X") return "(" + v + ").x";
        if (pinName == "Y") return "(" + v + ").y";
        if (pinName == "Z") return "(" + v + ").z";
        return "(" + v + ").x";
    }
};

// ============================================================================
// FRESNEL NODE - Fresnel effect
// ============================================================================
class FresnelNode : public ShaderNodeBase {
public:
    FresnelNode() {
        setTitle("Fresnel");
        setStyle(InputNodeStyle());
        addIN<ShaderCode>("Power", ShaderCode("2.0"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
        addOUT<ShaderCode>("Factor", FloatPinStyle())->behaviour([this]() {
            auto power = getInVal<ShaderCode>("Power");
            return ShaderCode("pow(1.0 - max(dot(normalize(Normal), normalize(viewPos - FragPos)), 0.0), " + power.code + ")");
        });
    }

    void draw() override {
        ImGui::Text("Fresnel Effect");
    }
    
    ShaderDataType getOutputType(const std::string& pinName) const override { return ShaderDataType::Float; }
    
    std::string generateExpression(const std::string& pinName,
                                   const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap) const override {
        std::string power = getInputVar(this, "Power", varMap, "2.0");
        return "pow(1.0 - max(dot(normalize(Normal), normalize(viewPos - FragPos)), 0.0), " + power + ")";
    }
};

// ============================================================================
// OUTPUT NODE - Final shader output (always needed)
// ============================================================================
class OutputNode : public ShaderNodeBase {
public:
    OutputNode() {
        setTitle("Shader Output");
        setStyle(OutputNodeStyle());
        addIN<ShaderCode>("Color", ShaderCode("vec3(1.0, 0.5, 0.2)"), ImFlow::ConnectionFilter::SameType(), Vec3PinStyle());
        addIN<ShaderCode>("Alpha", ShaderCode("1.0"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
    }

    void draw() override {
        ImGui::Text("Final Output");
    }
    
    // Legacy method - still works for simple graphs
    std::string generateCode() {
        auto color = getInVal<ShaderCode>("Color");
        auto alpha = getInVal<ShaderCode>("Alpha");
        return "    vec3 finalColor = " + color.code + ";\n"
               "    float finalAlpha = " + alpha.code + ";\n"
               "    FragColor = vec4(finalColor, finalAlpha);\n";
    }
    
    // New method using variable map from graph traversal
    std::string generateCodeFromVarMap(const std::unordered_map<ImFlow::NodeUID, std::unordered_map<std::string, std::string>>& varMap) const {
        std::string color = getInputVar(this, "Color", varMap, "vec3(1.0, 0.5, 0.2)");
        std::string alpha = getInputVar(this, "Alpha", varMap, "1.0");
        return "    vec3 finalColor = " + color + ";\n"
               "    float finalAlpha = " + alpha + ";\n"
               "    FragColor = vec4(finalColor, finalAlpha);\n";
    }
};

} // namespace ShaderGraph

#endif // SHADER_NODES_H
