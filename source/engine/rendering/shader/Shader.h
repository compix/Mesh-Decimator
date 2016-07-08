#pragma once
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

using ShaderProgram = GLuint;
using UniformName = std::string;

struct DirectionalLight
{
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 direction;
};

struct SMaterial
{
    glm::vec3 diffuse;
    glm::vec4 specular; // w = specular power
};

class Shader
{
public:
    Shader() {}
    Shader(const std::string& vsPath, const std::string& fsPath);
    ~Shader();

    void loadGS(const std::string& vsPath, const std::string& fsPath, const std::string& gsPath);
    void load(const std::string& vsPath, const std::string& fsPath, std::initializer_list<std::string> vertexAttributeNames = {});
    ShaderProgram getProgram() const { return m_shaderProgram; }

    void bind();

    /**
    * Sets the camera view and proj matrices "u_view" and "u_proj".
    */
    void setCamera(const glm::mat4& view, const glm::mat4& proj) const;

    /**
    * Sets the eye/camera position "u_eyePos".
    */
    void setEyePosition(const glm::vec3& pos);

    /**
    * Sets the model matrix "u_model".
    * If setInverseTranspose is true then "u_modelIT" 
    * will be set as the inverse transpose of the given model matrix.
    */
    void setModel(const glm::mat4& modelMatrix, bool setInverseTranspose = true) const;

    /**
    * Sets the 4 component color "u_color".
    */
    void setColor(float r, float g, float b, float a) const;

    /**
    * Sets the 4 component color "u_color".
    */
    void setColor(glm::vec4 color) const;

    /**
    * Sets the 3 component color "u_color".
    */
    void setColor(glm::vec3 color) const;

    /**
    * Sets the 3 component color "u_color".
    */
    void setColor(float r, float g, float b) const;

    /**
    * Sets the model view projection matrix "u_modelViewProj".
    */
    void setMVP(const glm::mat4& mvp) const;

    void setShaderProgram(ShaderProgram shaderProgram) { m_shaderProgram = shaderProgram; }

    bool hasSameProgram(ShaderProgram shaderProgram) const { return m_shaderProgram == shaderProgram; }

    void setFloat(const UniformName& uniformName, float v) const;
    void setDouble(const UniformName& uniformName, double v) const;
    void setInt(const UniformName& uniformName, int v) const;
    void setUnsignedInt(const UniformName& uniformName, glm::uint v) const;
    void setVector(const UniformName& uniformName, float v1, float v2) const;
    void setVector(const UniformName& uniformName, float v1, float v2, float v3) const;
    void setVector(const UniformName& uniformName, float v1, float v2, float v3, float v4) const;
    void setVector(const UniformName& uniformName, const glm::vec2& v) const;
    void setVector(const UniformName& uniformName, const glm::vec3& v) const;
    void setVector(const UniformName& uniformName, const glm::vec4& v) const;
    void setMatrix(const UniformName& uniformName, const glm::mat4& m) const;
    void setMatrix(const UniformName& uniformName, const glm::mat3& m) const;
    void setMatrix(const UniformName& uniformName, const glm::mat2& m) const;

    /**
    * textureName is the name of the texture in the shader.
    * textureUnit: Use 0 for the first sampler/texture, 1 for the second ... The order of samplers/textures in the shader doesn't matter.
    */
    void bindTexture2D(GLuint texId, const std::string& textureName, GLint textureUnit = 0);

    /**
    * textureName is the name of the texture in the shader.
    * textureUnit: Use 0 for the first sampler/texture, 1 for the second ... The order of samplers/textures in the shader doesn't matter.
    */
    void bindTexture3D(GLuint texId, const std::string& textureName, GLint textureUnit = 0);

    /**
    * Sets the directional light "u_dirLight" with the following members:
    * vec3 ambient;
	* vec3 diffuse;
	* vec3 specular;
	* vec3 viewDirection;
    */
    void setDirLight(const DirectionalLight& dirLight, const glm::mat4& view);

    /**
    * Sets the directional light "u_dirLight" with the following members:
    * vec3 ambient;
    * vec3 diffuse;
    * vec3 specular;
    * vec3 direction;
    */
    void setDirLight(const DirectionalLight& dirLight);

    /**
    * Sets the material "u_material" with the following members:
    * vec3 diffuse;
    * vec4 specular;
    */
    void setMaterial(const SMaterial& material);

    static void shaderErrorCheck(GLuint shader, const std::string& shaderPath);
    static void programErrorCheck(GLuint program, std::initializer_list<std::string> shaderPaths);
private:
    GLuint load(GLenum shaderType, const std::string& shaderPath);

    GLint getLocation(const char* uniformName) const
    {
        return glGetUniformLocation(m_shaderProgram, uniformName);
    }
private:
    ShaderProgram m_shaderProgram = 0;
    bool m_loadedProgram = false;
};

