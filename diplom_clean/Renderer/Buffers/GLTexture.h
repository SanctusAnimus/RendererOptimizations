#pragma once
#ifndef GLTEXTURE_STRUCT_DECL

#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct GLTexture {
	unsigned int m_Id = 0;
    unsigned int m_SlotUsed = 0;

    GLTexture() {
        glGenTextures(1, &m_Id);
    };

	GLTexture(int width, int height, unsigned int attachment) {
        glGenTextures(1, &m_Id);
        Set(width, height, attachment);
	}

    void Set(int width, int height, unsigned int attachment) {
        glBindTexture(GL_TEXTURE_2D, m_Id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, m_Id, 0);

        std::cout << this << " called set texture with ID: " << m_Id << std::endl;
    }

    void Use(unsigned int slot) {
        glActiveTexture(slot);
        glBindTexture(GL_TEXTURE_2D, m_Id);
        m_SlotUsed = slot;
    }

    void Reset() {
        glActiveTexture(m_SlotUsed);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    ~GLTexture() {
        std::cout << "gltexture destroyed ID: " << m_Id << std::endl;
        glDeleteTextures(1, &m_Id);
    }
};

#endif // !GLTEXTURE_STRUCT_DECL
