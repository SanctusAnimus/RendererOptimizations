#pragma once
#ifndef TEXTURE_STRUCT_DECL
#define TEXTURE_STRUCT_DECL

#include <string>

struct Texture {
    unsigned int id;
    unsigned int gl_type;
    std::string type;
    std::string path;  // we store the path of the texture to compare with other textures
};

#endif // !TEXTURE_STRUCT_DECL