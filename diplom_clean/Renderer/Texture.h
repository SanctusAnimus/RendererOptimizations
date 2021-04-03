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

    ~Texture() {
        std::cout << "destroying texture " << id << " " << path << std::endl;
        // glDeleteTextures(1, &id);
    }
};

#endif // !TEXTURE_STRUCT_DECL