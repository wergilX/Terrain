#pragma once

#include<string>
#include<GL/glew.h>
#include <glm/glm.hpp>

std::string get_file_contents(const char* fileName);

class Shader{
public:
    Shader( const char* vertexFile, const char* fragmentFile);
    void Activate();
    void Delete();
    
    void setBool(const std::string &name, bool value) const;  
    void setInt(const std::string &name, int value) const;   
    void setFloat(const std::string &name, float value) const;

    void setVec2(const std::string &name, const glm::vec2 &value) const;
    void setVec2(const std::string &name, float x, float y) const;

    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec3(const std::string &name, float x, float y, float z) const;
    
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setVec4(const std::string &name, float x, float y, float z, float w) const;
    
    void setMat2(const std::string &name, const glm::mat2 &mat) const;
    
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

public:
    GLuint ID;
};
