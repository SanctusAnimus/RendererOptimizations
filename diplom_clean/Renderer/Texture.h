#pragma once
#ifndef TEXTURE_STRUCT_DECL
#define TEXTURE_STRUCT_DECL

#include <string>
#include <glad/glad.h>

struct Texture {
    unsigned int id;
    unsigned int gl_type;
    std::string type;
    std::string path;  // we store the path of the texture to compare with other textures

    ~Texture() {
        // glDeleteTextures(1, &id);
    }
};

#endif // !TEXTURE_STRUCT_DECL