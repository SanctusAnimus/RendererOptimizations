#pragma once
#ifndef TEXTURE_STRUCT_DECL
#define TEXTURE_STRUCT_DECL

#include <string>
#include <iostream>
#include <glad/glad.h>

struct Texture {
    unsigned int id;
    unsigned int gl_type;
    std::string type;
    std::string path;  // we store the path of the texture to compare with other textures

    Texture(unsigned int id, unsigned int gl_type, std::string type, std::string path) 
        : id(id), gl_type(gl_type), type(type), path(path)
    {
        std::cout << "[Texture] constructor called!" << std::endl;
    }

    ~Texture() {
        std::cout << "[Texture] destroying " << id << " " << path << std::endl;
        glDeleteTextures(1, &id);
    }
};

#endif // !TEXTURE_STRUCT_DECL