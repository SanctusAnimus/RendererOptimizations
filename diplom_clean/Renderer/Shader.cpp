#include "Shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath, std::string name) : m_Name(name)
{
    std::cout << "[Shader] Creating shader type VS + FS" << std::endl;
    auto vertex = addShader(vertexPath, GL_VERTEX_SHADER, "VERTEX");
    auto fragment = addShader(fragmentPath, GL_FRAGMENT_SHADER, "FRAGMENT");

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    std::cout << "[Shader] ...Done" << std::endl;
}


Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath, std::string name) : m_Name(name)
{
    std::cout << "[Shader] Creating shader type VS + FS + GS" << std::endl;
    auto vertex = addShader(vertexPath, GL_VERTEX_SHADER, "VERTEX");
    auto fragment = addShader(fragmentPath, GL_FRAGMENT_SHADER, "FRAGMENT");
    auto geometry = addShader(geometryPath, GL_GEOMETRY_SHADER, "GEOMETRY");

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glAttachShader(ID, geometry);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    glDeleteShader(geometry);
    std::cout << "[Shader] ...Done" << std::endl;
}


Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* tessControlPath, const char* tessEvalPath, std::string name) : m_Name(name)
{
    std::cout << "[Shader] Creating shader type VS + FS + TCS + TES" << std::endl;
    auto vertex = addShader(vertexPath, GL_VERTEX_SHADER, "VERTEX");
    auto fragment = addShader(fragmentPath, GL_FRAGMENT_SHADER, "FRAGMENT");
    auto tess_control = addShader(tessControlPath, GL_TESS_CONTROL_SHADER, "TESS_CONTROL");
    auto tess_eval = addShader(tessEvalPath, GL_TESS_EVALUATION_SHADER, "TESS_EVAL");
    // auto geometry = addShader("Shaders/Tessellation/tessellation_geometry.glsl", GL_GEOMETRY_SHADER, "GEOMETRY");

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glAttachShader(ID, tess_control);
    glAttachShader(ID, tess_eval);
    // glAttachShader(ID, geometry);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    glDeleteShader(tess_control);
    glDeleteShader(tess_eval);
    // glDeleteShader(geometry);
    std::cout << "[Shader] ...Done" << std::endl;
}

void Shader::use() 
{
    glUseProgram(this->ID);
}

void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec2(const std::string& name, float x, float y) const
{
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}
 
void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}
 
void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}
 
void Shader::setMat2(const std::string& name, const glm::mat2& mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
 
void Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
 
void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}


void Shader::checkCompileErrors(unsigned int shader, std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "\tERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n[END]" << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "\tERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n[END]" << std::endl;
        }
    }
}


unsigned int Shader::addShader(const char* path, unsigned int shader_type, const std::string& s_type_repr) {
    std::cout << "\tadding shader type <" << s_type_repr << "> from " << path << std::endl;
    // 1. retrieve the vertex/fragment source code from filePath
    std::string shader_code;
    std::ifstream shader_file;
    // ensure ifstream objects can throw exceptions:
    shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        shader_file.open(path);
        std::stringstream shader_stream;
        // read file's buffer contents into streams
        shader_stream << shader_file.rdbuf();
        // close file handlers
        shader_file.close();
        // convert stream into string
        shader_code = shader_stream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "\tERROR::SHADER::FILE_NOT_SUCCESFULLY_READ:" << s_type_repr << std::endl;
    }
    const char* c_shader_code = shader_code.c_str();
    // 2. compile shaders
    unsigned int shader_id;
    // vertex shader
    shader_id = glCreateShader(shader_type);
    glShaderSource(shader_id, 1, &c_shader_code, NULL);
    glCompileShader(shader_id);
    checkCompileErrors(shader_id, s_type_repr);
    return shader_id;
}