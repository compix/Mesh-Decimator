#include "Shader.h"
#include <GL/glew.h>
#include <string>
#include <iostream>
#include <vector>
#include <engine/util/Logger.h>
#include <engine/util/file.h>

void Shader::shaderErrorCheck(GLuint shader, const std::string& shaderPath)
{
    GLint result = GL_FALSE;
    GLint infoLogLength;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<char> shaderLog;

    if (infoLogLength > 0)
    {
        shaderLog.resize(infoLogLength);
        glGetShaderInfoLog(shader, infoLogLength, nullptr, &shaderLog[0]);
    }

    if (result == GL_FALSE)
        std::cout << "Shader comilation failed for " << shaderPath << std::endl;

    if (shaderLog.size() > 0)
    {
        std::cout << shaderPath << ": \n";
        fprintf(stdout, "%s\n", &shaderLog[0]);
    }
}

void Shader::programErrorCheck(GLuint program, std::initializer_list<std::string> shaderPaths)
{
    GLint result = GL_FALSE;
    GLint infoLogLength;

    glGetProgramiv(program, GL_LINK_STATUS, &result);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<char> programLog;
    if (infoLogLength > 0)
    {
        programLog.resize(infoLogLength);
        glGetProgramInfoLog(program, infoLogLength, nullptr, &programLog[0]);
    }

    if (result == GL_FALSE)
    {
        std::cout << "Linking the program failed for ";
        for (auto& shaderPath : shaderPaths)
            std::cout << shaderPath << " ";
        std::cout << "\n";
    }

    if (programLog.size() > 0)
    {
        std::cout << "Program Log - ";
        for (auto& shaderPath : shaderPaths)
            std::cout << shaderPath << " ";
        std::cout << std::endl;
        fprintf(stdout, "%s\n", &programLog[0]);
    }
}

void Shader::load(const std::string& vsPath, const std::string& fsPath, std::initializer_list<std::string> vertexAttributeNames)
{
    if (m_loadedProgram)
        glDeleteProgram(m_shaderProgram);

    GLuint vsID = load(GL_VERTEX_SHADER, vsPath);
    GLuint fsID = load(GL_FRAGMENT_SHADER, fsPath);

    GLuint program = glCreateProgram();

    glAttachShader(program, vsID);
    glAttachShader(program, fsID);

    uint8_t i = 0;
    for (auto& n : vertexAttributeNames)
    {
        glBindAttribLocation(program, GLuint(i), n.c_str());
        ++i;
    }

    glLinkProgram(program);

    programErrorCheck(program, { vsPath, fsPath });

    glDeleteShader(vsID);
    glDeleteShader(fsID);

    GL_ERROR_CHECK();

    m_shaderProgram = program;
    m_loadedProgram = true;
}

Shader::Shader(const std::string& vsPath, const std::string& fsPath)
{
    load(vsPath, fsPath);
}

Shader::~Shader()
{
    if (m_loadedProgram)
        glDeleteProgram(m_shaderProgram);
}

void Shader::loadGS(const std::string& vsPath, const std::string& fsPath, const std::string& gsPath)
{
    if (m_loadedProgram)
        glDeleteProgram(m_shaderProgram);

    GLuint vsID = load(GL_VERTEX_SHADER, vsPath);
    GLuint fsID = load(GL_FRAGMENT_SHADER, fsPath);
    GLuint gsID = load(GL_GEOMETRY_SHADER, gsPath);

    GLuint program = glCreateProgram();

    glAttachShader(program, vsID);
    glAttachShader(program, fsID);
    glAttachShader(program, gsID);

    glLinkProgram(program);

    programErrorCheck(program, { vsPath, fsPath, gsPath });

    glDeleteShader(vsID);
    glDeleteShader(fsID);
    glDeleteShader(gsID);

    GL_ERROR_CHECK();

    m_shaderProgram = program;
    m_loadedProgram = true;
}

void Shader::bindTexture2D(GLuint texId, const std::string& textureName, GLint textureUnit)
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, texId);
    glUniform1i(glGetUniformLocation(m_shaderProgram, textureName.c_str()), textureUnit);
}

void Shader::bindTexture3D(GLuint texId, const std::string& textureName, GLint textureUnit)
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_3D, texId);
    glUniform1i(glGetUniformLocation(m_shaderProgram, textureName.c_str()), textureUnit);
}

void Shader::setDirLight(const DirectionalLight& dirLight, const glm::mat4& view)
{
    setVector("u_dirLight.ambient", dirLight.ambient);
    setVector("u_dirLight.diffuse", dirLight.diffuse);
    setVector("u_dirLight.specular", dirLight.specular);
    setVector("u_dirLight.viewDirection", glm::vec3(view * glm::vec4(dirLight.direction, 0.0f)));
}

void Shader::setDirLight(const DirectionalLight& dirLight)
{
    setVector("u_dirLight.ambient", dirLight.ambient);
    setVector("u_dirLight.diffuse", dirLight.diffuse);
    setVector("u_dirLight.specular", dirLight.specular);
    setVector("u_dirLight.direction", dirLight.direction);
}

void Shader::setMaterial(const SMaterial& material)
{
    setVector("u_material.diffuse", material.diffuse);
    setVector("u_material.specular", material.specular);
}

GLuint Shader::load(GLenum shaderType, const std::string& shaderPath)
{
    GLuint id = glCreateShader(shaderType);

    std::string source = file::readAsString(shaderPath);
    char const* cSource = source.c_str();
    glShaderSource(id, 1, &cSource, nullptr);
    glCompileShader(id);

    shaderErrorCheck(id, shaderPath);

    return id;
}

void Shader::bind()
{
    glUseProgram(m_shaderProgram);
}

void Shader::setCamera(const glm::mat4& view, const glm::mat4& proj) const
{
    glUniformMatrix4fv(getLocation("u_view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(getLocation("u_proj"), 1, GL_FALSE, &proj[0][0]);
}

void Shader::setEyePosition(const glm::vec3& pos)
{
    setVector("u_eyePos", pos);
}

void Shader::setModel(const glm::mat4& modelMatrix, bool setInverseTranspose) const
{
    glUniformMatrix4fv(getLocation("u_model"), 1, GL_FALSE, &modelMatrix[0][0]);

    if (setInverseTranspose)
    {
        glm::mat4 modelIT = glm::transpose(glm::inverse(modelMatrix));
        glUniformMatrix4fv(getLocation("u_modelIT"), 1, GL_FALSE, &modelIT[0][0]);
    }
}

void Shader::setColor(float r, float g, float b, float a) const
{
    glUniform4f(getLocation("u_color"), r, g, b, a);
}

void Shader::setColor(glm::vec4 color) const
{
    glUniform4f(getLocation("u_color"), color.r, color.g, color.b, color.a);
}

void Shader::setColor(glm::vec3 color) const
{
    setColor(color.r, color.g, color.b);
}

void Shader::setColor(float r, float g, float b) const
{
    glUniform3f(getLocation("u_color"), r, g, b);
}

void Shader::setMVP(const glm::mat4& mvp) const
{
    glUniformMatrix4fv(getLocation("u_modelViewProj"), 1, GL_FALSE, &mvp[0][0]);
}

void Shader::setFloat(const UniformName& uniformName, float v) const
{
    glUniform1f(getLocation(uniformName.c_str()), v);
}

void Shader::setDouble(const UniformName& uniformName, double v) const
{
    glUniform1d(getLocation(uniformName.c_str()), v);
}

void Shader::setInt(const UniformName& uniformName, int v) const
{
    glUniform1i(getLocation(uniformName.c_str()), v);
}

void Shader::setUnsignedInt(const UniformName& uniformName, glm::uint v) const
{
    glUniform1ui(getLocation(uniformName.c_str()), v);
}

void Shader::setVector(const UniformName& uniformName, float v1, float v2) const
{
    glUniform2f(getLocation(uniformName.c_str()), v1, v2);
}

void Shader::setVector(const UniformName& uniformName, float v1, float v2, float v3) const
{
    glUniform3f(getLocation(uniformName.c_str()), v1, v2, v3);
}

void Shader::setVector(const UniformName& uniformName, float v1, float v2, float v3, float v4) const
{
    glUniform4f(getLocation(uniformName.c_str()), v1, v2, v3, v4);
}

void Shader::setVector(const UniformName& uniformName, const glm::vec2& v) const
{
    glUniform2f(getLocation(uniformName.c_str()), v.x, v.y);
}

void Shader::setVector(const UniformName& uniformName, const glm::vec3& v) const
{
    glUniform3f(getLocation(uniformName.c_str()), v.x, v.y, v.z);
}

void Shader::setVector(const UniformName& uniformName, const glm::vec4& v) const
{
    glUniform4f(getLocation(uniformName.c_str()), v.x, v.y, v.z, v.w);
}

void Shader::setMatrix(const UniformName& uniformName, const glm::mat4& m) const
{
    glUniformMatrix4fv(getLocation(uniformName.c_str()), 1, GL_FALSE, &m[0][0]);
}

void Shader::setMatrix(const UniformName& uniformName, const glm::mat3& m) const
{
    glUniformMatrix3fv(getLocation(uniformName.c_str()), 1, GL_FALSE, &m[0][0]);
}

void Shader::setMatrix(const UniformName& uniformName, const glm::mat2& m) const
{
    glUniformMatrix2fv(getLocation(uniformName.c_str()), 1, GL_FALSE, &m[0][0]);
}
