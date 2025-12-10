#ifndef SHADER_NODES_H
#define SHADER_NODES_H

#include "ImNodeFlow.h"
#include <string>
#include <sstream>
#include <iomanip>

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

// ============================================================================
// FLOAT CONSTANT NODE - Outputs a constant float value
// ============================================================================
class FloatNode : public ImFlow::BaseNode {
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

private:
    float m_value = 0.0f;
};

// ============================================================================
// VEC3 COLOR NODE - Outputs a vec3 color value
// ============================================================================
class ColorNode : public ImFlow::BaseNode {
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

private:
    float m_color[3] = {1.0f, 0.5f, 0.2f};
};

// ============================================================================
// TIME NODE - Outputs the time uniform
// ============================================================================
class TimeNode : public ImFlow::BaseNode {
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
};

// ============================================================================
// UV NODE - Outputs UV coordinates (FragPos based)
// ============================================================================
class UVNode : public ImFlow::BaseNode {
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
};

// ============================================================================
// NORMAL NODE - Outputs the normal vector
// ============================================================================
class NormalNode : public ImFlow::BaseNode {
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
};

// ============================================================================
// ADD NODE - Adds two values
// ============================================================================
class AddNode : public ImFlow::BaseNode {
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
};

// ============================================================================
// MULTIPLY NODE - Multiplies two values
// ============================================================================
class MultiplyNode : public ImFlow::BaseNode {
public:
    MultiplyNode() {
        setTitle("Multiply");
        setStyle(MathNodeStyle());
        addIN<ShaderCode>("A", ShaderCode("1.0"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
        addIN<ShaderCode>("B", ShaderCode("1.0"), ImFlow::ConnectionFilter::SameType(), FloatPinStyle());
        addOUT<ShaderCode>("Result", FloatPinStyle())->behaviour([this]() {
            auto a = getInVal<ShaderCode>("A");
            auto b = getInVal<ShaderCode>("B");
            return ShaderCode("(" + a.code + " * " + b.code + ")");
        });
    }

    void draw() override {
        ImGui::Text("A * B");
    }
};

// ============================================================================
// SUBTRACT NODE - Subtracts two values
// ============================================================================
class SubtractNode : public ImFlow::BaseNode {
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
};

// ============================================================================
// DIVIDE NODE - Divides two values
// ============================================================================
class DivideNode : public ImFlow::BaseNode {
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
};

// ============================================================================
// SIN NODE - Sine function
// ============================================================================
class SinNode : public ImFlow::BaseNode {
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
};

// ============================================================================
// COS NODE - Cosine function
// ============================================================================
class CosNode : public ImFlow::BaseNode {
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
};

// ============================================================================
// ABS NODE - Absolute value
// ============================================================================
class AbsNode : public ImFlow::BaseNode {
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
};

// ============================================================================
// MIX NODE - Linear interpolation
// ============================================================================
class MixNode : public ImFlow::BaseNode {
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
};

// ============================================================================
// CLAMP NODE - Clamp value
// ============================================================================
class ClampNode : public ImFlow::BaseNode {
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

private:
    float m_min = 0.0f;
    float m_max = 1.0f;
};

// ============================================================================
// MAKE VEC3 NODE - Creates a vec3 from components
// ============================================================================
class MakeVec3Node : public ImFlow::BaseNode {
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
};

// ============================================================================
// SPLIT VEC3 NODE - Splits a vec3 into components
// ============================================================================
class SplitVec3Node : public ImFlow::BaseNode {
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
};

// ============================================================================
// FRESNEL NODE - Fresnel effect
// ============================================================================
class FresnelNode : public ImFlow::BaseNode {
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
};

// ============================================================================
// OUTPUT NODE - Final shader output (always needed)
// ============================================================================
class OutputNode : public ImFlow::BaseNode {
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
    
    std::string generateCode() {
        auto color = getInVal<ShaderCode>("Color");
        auto alpha = getInVal<ShaderCode>("Alpha");
        return "    vec3 finalColor = " + color.code + ";\n"
               "    float finalAlpha = " + alpha.code + ";\n"
               "    FragColor = vec4(finalColor, finalAlpha);\n";
    }
};

} // namespace ShaderGraph

#endif // SHADER_NODES_H
