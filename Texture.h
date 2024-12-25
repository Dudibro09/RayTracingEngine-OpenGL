#pragma once
#ifndef FRAMEBUFFER_TEXTURE_CLASS_H
#define FRAMEBUFFER_TEXTURE_CLASS_H

#include <glad/glad.h>
#include <iostream>
#include <string>
#include <vector>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

// A simple 32bit float RGB texture format
class Texture
{
private:
	int m_width, m_height;
	GLenum m_slot;

public:
	GLuint ID;

	Texture();

	void Initialize(GLenum slot);
	void Delete();
	void Bind() const;
	void Unbind() const;

	void Resize(int width, int height);
	void LoadFromFile(const char* file);

	void GetTextureData(int& width, int& height, std::vector<float>& data) const;
	void SetTextureData(int width, int height, float* data);
	void UploadToShader(const char* uniform, GLuint shaderID, GLint s) const;
};

#endif