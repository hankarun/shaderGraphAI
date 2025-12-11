#ifndef SHADER_LANG_H
#define SHADER_LANG_H

#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <regex>
#include <algorithm>

namespace ShaderGraph {

// Supported shader language backends
enum class ShaderLanguage {
    HLSL,   // DirectX 11/12
    GLSL    // OpenGL 3.3+
};

// Shader model/version info
struct ShaderVersion {
    ShaderLanguage language;
    int majorVersion;
    int minorVersion;
    
    static ShaderVersion HLSL_5_0() { return {ShaderLanguage::HLSL, 5, 0}; }
    static ShaderVersion GLSL_330() { return {ShaderLanguage::GLSL, 3, 30}; }
    static ShaderVersion GLSL_450() { return {ShaderLanguage::GLSL, 4, 50}; }
};

// Cross-platform shader type names
class ShaderTypes {
public:
    static std::string Float(ShaderLanguage lang) {
        return "float";  // Same in both
    }
    
    static std::string Float2(ShaderLanguage lang) {
        return (lang == ShaderLanguage::HLSL) ? "float2" : "vec2";
    }
    
    static std::string Float3(ShaderLanguage lang) {
        return (lang == ShaderLanguage::HLSL) ? "float3" : "vec3";
    }
    
    static std::string Float4(ShaderLanguage lang) {
        return (lang == ShaderLanguage::HLSL) ? "float4" : "vec4";
    }
    
    static std::string Matrix4x4(ShaderLanguage lang) {
        return (lang == ShaderLanguage::HLSL) ? "float4x4" : "mat4";
    }
    
    static std::string Matrix3x3(ShaderLanguage lang) {
        return (lang == ShaderLanguage::HLSL) ? "float3x3" : "mat3";
    }
    
    static std::string Sampler2D(ShaderLanguage lang) {
        return (lang == ShaderLanguage::HLSL) ? "Texture2D" : "sampler2D";
    }
};

// Cross-platform shader function names
class ShaderFunctions {
public:
    // Math functions
    static std::string Lerp(ShaderLanguage lang) {
        return (lang == ShaderLanguage::HLSL) ? "lerp" : "mix";
    }
    
    static std::string Saturate(ShaderLanguage lang) {
        // GLSL doesn't have saturate, need to use clamp
        return (lang == ShaderLanguage::HLSL) ? "saturate" : "clamp";
    }
    
    static std::string Frac(ShaderLanguage lang) {
        return (lang == ShaderLanguage::HLSL) ? "frac" : "fract";
    }
    
    static std::string Atan2(ShaderLanguage lang) {
        return (lang == ShaderLanguage::HLSL) ? "atan2" : "atan";
    }
    
    static std::string Ddx(ShaderLanguage lang) {
        return (lang == ShaderLanguage::HLSL) ? "ddx" : "dFdx";
    }
    
    static std::string Ddy(ShaderLanguage lang) {
        return (lang == ShaderLanguage::HLSL) ? "ddy" : "dFdy";
    }
    
    // These are the same in both languages
    static std::string Sin() { return "sin"; }
    static std::string Cos() { return "cos"; }
    static std::string Tan() { return "tan"; }
    static std::string Pow() { return "pow"; }
    static std::string Sqrt() { return "sqrt"; }
    static std::string Abs() { return "abs"; }
    static std::string Floor() { return "floor"; }
    static std::string Ceil() { return "ceil"; }
    static std::string Round() { return "round"; }
    static std::string Min() { return "min"; }
    static std::string Max() { return "max"; }
    static std::string Clamp() { return "clamp"; }
    static std::string Step() { return "step"; }
    static std::string Smoothstep() { return "smoothstep"; }
    static std::string Length() { return "length"; }
    static std::string Distance() { return "distance"; }
    static std::string Normalize() { return "normalize"; }
    static std::string Dot() { return "dot"; }
    static std::string Cross() { return "cross"; }
    static std::string Reflect() { return "reflect"; }
    static std::string Refract() { return "refract"; }
};

// HLSL to GLSL converter
class HLSLtoGLSLConverter {
public:
    static std::string convert(const std::string& hlslCode) {
        std::string result = hlslCode;
        
        // Convert types
        result = convertTypes(result);
        
        // Convert functions
        result = convertFunctions(result);
        
        // Convert semantics and structure
        result = convertSemantics(result);
        
        // Convert texture sampling
        result = convertTextureSampling(result);
        
        // Convert swizzle patterns (if needed)
        result = convertSwizzle(result);
        
        return result;
    }
    
private:
    static std::string convertTypes(const std::string& code) {
        std::string result = code;
        
        // Type conversions (order matters - longer names first)
        static const std::vector<std::pair<std::string, std::string>> typeMap = {
            {"float4x4", "mat4"},
            {"float3x3", "mat3"},
            {"float2x2", "mat2"},
            {"float4", "vec4"},
            {"float3", "vec3"},
            {"float2", "vec2"},
            {"half4", "vec4"},
            {"half3", "vec3"},
            {"half2", "vec2"},
            {"half", "float"},
            {"int4", "ivec4"},
            {"int3", "ivec3"},
            {"int2", "ivec2"},
            {"uint4", "uvec4"},
            {"uint3", "uvec3"},
            {"uint2", "uvec2"},
            {"bool4", "bvec4"},
            {"bool3", "bvec3"},
            {"bool2", "bvec2"},
        };
        
        for (const auto& [hlsl, glsl] : typeMap) {
            result = replaceWord(result, hlsl, glsl);
        }
        
        return result;
    }
    
    static std::string convertFunctions(const std::string& code) {
        std::string result = code;
        
        // Function conversions
        static const std::vector<std::pair<std::string, std::string>> funcMap = {
            {"lerp", "mix"},
            {"frac", "fract"},
            {"ddx", "dFdx"},
            {"ddy", "dFdy"},
            {"ddx_coarse", "dFdx"},
            {"ddy_coarse", "dFdy"},
            {"ddx_fine", "dFdx"},
            {"ddy_fine", "dFdy"},
            {"atan2", "atan"},
            {"rsqrt", "inversesqrt"},
            {"fmod", "mod"},
            {"clip", "discard"},  // Special handling needed
            {"mul", "matrixCompMult"},  // Simplified - actual mul needs special handling
        };
        
        for (const auto& [hlsl, glsl] : funcMap) {
            result = replaceWord(result, hlsl, glsl);
        }
        
        // Handle saturate -> clamp(x, 0.0, 1.0)
        result = convertSaturate(result);
        
        return result;
    }
    
    static std::string convertSaturate(const std::string& code) {
        std::string result = code;
        std::regex saturateRegex(R"(saturate\s*\(\s*([^)]+)\s*\))");
        result = std::regex_replace(result, saturateRegex, "clamp($1, 0.0, 1.0)");
        return result;
    }
    
    static std::string convertSemantics(const std::string& code) {
        std::string result = code;
        
        // Remove HLSL semantics like : POSITION, : TEXCOORD0, : SV_Target, etc.
        // This is a simplified version - full conversion needs more context
        std::regex semanticRegex(R"(\s*:\s*(SV_\w+|POSITION\d*|TEXCOORD\d*|NORMAL\d*|COLOR\d*|TANGENT\d*|BINORMAL\d*))");
        result = std::regex_replace(result, semanticRegex, "");
        
        return result;
    }
    
    static std::string convertTextureSampling(const std::string& code) {
        std::string result = code;
        
        // tex2D(sampler, uv) -> texture(sampler, uv)
        std::regex tex2DRegex(R"(tex2D\s*\()");
        result = std::regex_replace(result, tex2DRegex, "texture(");
        
        // Sample(sampler, uv) -> texture(sampler, uv) for Texture2D
        // This is simplified - full conversion needs texture/sampler pairs
        
        return result;
    }
    
    static std::string convertSwizzle(const std::string& code) {
        // HLSL and GLSL use the same swizzle syntax (xyzw, rgba)
        // No conversion needed for basic cases
        return code;
    }
    
    // Replace whole words only (not partial matches)
    static std::string replaceWord(const std::string& text, const std::string& from, const std::string& to) {
        std::string result = text;
        std::regex wordRegex("\\b" + from + "\\b");
        result = std::regex_replace(result, wordRegex, to);
        return result;
    }
};

// GLSL to HLSL converter (for completeness)
class GLSLtoHLSLConverter {
public:
    static std::string convert(const std::string& glslCode) {
        std::string result = glslCode;
        
        // Convert types
        result = convertTypes(result);
        
        // Convert functions  
        result = convertFunctions(result);
        
        return result;
    }
    
private:
    static std::string convertTypes(const std::string& code) {
        std::string result = code;
        
        static const std::vector<std::pair<std::string, std::string>> typeMap = {
            {"mat4", "float4x4"},
            {"mat3", "float3x3"},
            {"mat2", "float2x2"},
            {"vec4", "float4"},
            {"vec3", "float3"},
            {"vec2", "float2"},
            {"ivec4", "int4"},
            {"ivec3", "int3"},
            {"ivec2", "int2"},
            {"uvec4", "uint4"},
            {"uvec3", "uint3"},
            {"uvec2", "uint2"},
            {"bvec4", "bool4"},
            {"bvec3", "bool3"},
            {"bvec2", "bool2"},
        };
        
        for (const auto& [glsl, hlsl] : typeMap) {
            result = replaceWord(result, glsl, hlsl);
        }
        
        return result;
    }
    
    static std::string convertFunctions(const std::string& code) {
        std::string result = code;
        
        static const std::vector<std::pair<std::string, std::string>> funcMap = {
            {"mix", "lerp"},
            {"fract", "frac"},
            {"dFdx", "ddx"},
            {"dFdy", "ddy"},
            {"inversesqrt", "rsqrt"},
            {"mod", "fmod"},
        };
        
        for (const auto& [glsl, hlsl] : funcMap) {
            result = replaceWord(result, glsl, hlsl);
        }
        
        // Handle clamp(x, 0.0, 1.0) -> saturate(x) where applicable
        // This is optional optimization
        
        return result;
    }
    
    static std::string replaceWord(const std::string& text, const std::string& from, const std::string& to) {
        std::string result = text;
        std::regex wordRegex("\\b" + from + "\\b");
        result = std::regex_replace(result, wordRegex, to);
        return result;
    }
};

// Cross-platform shader code builder
class ShaderCodeBuilder {
public:
    ShaderCodeBuilder(ShaderLanguage lang = ShaderLanguage::HLSL) : m_language(lang) {}
    
    void setLanguage(ShaderLanguage lang) { m_language = lang; }
    ShaderLanguage getLanguage() const { return m_language; }
    
    // Type helpers
    std::string Float() const { return ShaderTypes::Float(m_language); }
    std::string Float2() const { return ShaderTypes::Float2(m_language); }
    std::string Float3() const { return ShaderTypes::Float3(m_language); }
    std::string Float4() const { return ShaderTypes::Float4(m_language); }
    
    // Constructor helpers - create type with values
    std::string Float2(const std::string& x, const std::string& y) const {
        return ShaderTypes::Float2(m_language) + "(" + x + ", " + y + ")";
    }
    
    std::string Float3(const std::string& x, const std::string& y, const std::string& z) const {
        return ShaderTypes::Float3(m_language) + "(" + x + ", " + y + ", " + z + ")";
    }
    
    std::string Float4(const std::string& x, const std::string& y, const std::string& z, const std::string& w) const {
        return ShaderTypes::Float4(m_language) + "(" + x + ", " + y + ", " + z + ", " + w + ")";
    }
    
    // Function helpers
    std::string Lerp(const std::string& a, const std::string& b, const std::string& t) const {
        return ShaderFunctions::Lerp(m_language) + "(" + a + ", " + b + ", " + t + ")";
    }
    
    std::string Saturate(const std::string& x) const {
        if (m_language == ShaderLanguage::HLSL) {
            return "saturate(" + x + ")";
        } else {
            return "clamp(" + x + ", 0.0, 1.0)";
        }
    }
    
    std::string Frac(const std::string& x) const {
        return ShaderFunctions::Frac(m_language) + "(" + x + ")";
    }

private:
    ShaderLanguage m_language;
};

// Complete shader generator with multi-language support
class CrossPlatformShaderGenerator {
public:
    CrossPlatformShaderGenerator() : m_primaryLanguage(ShaderLanguage::HLSL) {}
    
    void setPrimaryLanguage(ShaderLanguage lang) { m_primaryLanguage = lang; }
    ShaderLanguage getPrimaryLanguage() const { return m_primaryLanguage; }
    
    // Generate shader in primary language (HLSL)
    std::string generateHLSL(const std::string& shaderBody, 
                             const std::vector<std::pair<std::string, std::string>>& uniforms) {
        std::stringstream ss;
        
        // HLSL header with cbuffer
        ss << "// Generated HLSL Shader\n";
        ss << "// Shader Model 5.0\n\n";
        
        // Constant buffer for uniforms
        ss << "cbuffer PerFrame : register(b0)\n{\n";
        ss << "    float time;\n";
        ss << "    float3 lightPos;\n";
        ss << "    float3 viewPos;\n";
        ss << "    float3 lightColor;\n";
        ss << "    float3 objectColor;\n";
        ss << "};\n\n";
        
        // User uniforms
        if (!uniforms.empty()) {
            ss << "cbuffer PerMaterial : register(b1)\n{\n";
            for (const auto& [type, name] : uniforms) {
                ss << "    " << type << " " << name << ";\n";
            }
            ss << "};\n\n";
        }
        
        // Input structure
        ss << "struct PSInput\n{\n";
        ss << "    float4 position : SV_POSITION;\n";
        ss << "    float3 fragPos : TEXCOORD0;\n";
        ss << "    float3 normal : NORMAL;\n";
        ss << "};\n\n";
        
        // Pixel shader
        ss << "float4 PSMain(PSInput input) : SV_TARGET\n{\n";
        ss << "    float3 FragPos = input.fragPos;\n";
        ss << "    float3 Normal = input.normal;\n\n";
        ss << shaderBody;
        ss << "}\n";
        
        return ss.str();
    }
    
    // Generate shader in GLSL (converted from HLSL or directly)
    std::string generateGLSL(const std::string& shaderBody,
                             const std::vector<std::pair<std::string, std::string>>& uniforms) {
        std::stringstream ss;
        
        // GLSL header
        ss << "#version 330 core\n";
        ss << "// Generated GLSL Shader (converted from HLSL)\n\n";
        
        ss << "out vec4 FragColor;\n\n";
        ss << "in vec3 FragPos;\n";
        ss << "in vec3 Normal;\n\n";
        
        // Built-in uniforms
        ss << "uniform float time;\n";
        ss << "uniform vec3 lightPos;\n";
        ss << "uniform vec3 viewPos;\n";
        ss << "uniform vec3 lightColor;\n";
        ss << "uniform vec3 objectColor;\n\n";
        
        // User uniforms (convert types)
        for (const auto& [type, name] : uniforms) {
            std::string glslType = type;
            // Convert HLSL types to GLSL
            if (type == "float3") glslType = "vec3";
            else if (type == "float2") glslType = "vec2";
            else if (type == "float4") glslType = "vec4";
            ss << "uniform " << glslType << " " << name << ";\n";
        }
        if (!uniforms.empty()) ss << "\n";
        
        ss << "void main()\n{\n";
        
        // Convert HLSL shader body to GLSL
        std::string glslBody = HLSLtoGLSLConverter::convert(shaderBody);
        ss << glslBody;
        
        ss << "}\n";
        
        return ss.str();
    }
    
    // Get shader in requested language
    std::string getShader(ShaderLanguage targetLang,
                          const std::string& shaderBody,
                          const std::vector<std::pair<std::string, std::string>>& uniforms) {
        if (targetLang == ShaderLanguage::HLSL) {
            return generateHLSL(shaderBody, uniforms);
        } else {
            return generateGLSL(shaderBody, uniforms);
        }
    }

private:
    ShaderLanguage m_primaryLanguage;
};

} // namespace ShaderGraph

#endif // SHADER_LANG_H
