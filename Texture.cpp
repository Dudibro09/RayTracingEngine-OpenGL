#include "Texture.h"

Texture::Texture() :
	ID(0xffffffff),
	m_slot(0xffffffff),
	m_width(0),
	m_height(0)
{}

void Texture::Initialize(GLenum slot)
{
	Texture::m_slot = slot;

	// Create Framebuffer Texture
	glGenTextures(1, &ID);
	// Bind the texture to the slot
	Bind();

	// Set the texture settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::Delete()
{
	glDeleteTextures(1, &ID);
}

void Texture::Bind() const
{
	// Bind the slot
	glActiveTexture(m_slot);
	// Bind the texture
	glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Resize(int w, int h)
{
	m_width = w;
	m_height = h;

	// Set this texture current
	Bind();
	// Resize the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	// Unbind the texture to not accidentally make changes to it
	Unbind();
}

void Texture::LoadFromFile(const char* file)
{
	int bpp = 0;
	unsigned char* data = stbi_load(file, &m_width, &m_height, &bpp, 3);
	if (!data)
	{
		throw std::string("Failed to load image");
		return;
	}

	// Convert the image to a normalized array of floats
	std::vector<float> dataNormalized(m_width * m_height * 3, 0.0f);
	for (int i = 0; i < m_width * m_height * 3; i++)
	{
		dataNormalized[i] = (float)data[i] / 255.0f;
	}

	stbi_image_free(data);

	SetTextureData(m_width, m_height, dataNormalized.data());
}

void Texture::GetTextureData(int& w, int& h, std::vector<float>& data) const
{
	w = m_width;
	h = m_height;
	data.resize(m_width * m_height * 3);

	// Set this texture current
	Bind();
	// Download the image data
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, data.data());
	// Unbind the texture to not accidentally make changes to it
	Unbind();
}

void Texture::SetTextureData(int w, int h, float* data)
{
	m_width = w;
	m_height = h;

	// Set this texture current
	Bind();
	// Upload the texture data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_width, m_height, 0, GL_RGB, GL_FLOAT, data);
	// Unbind the texture to not accidentally make changes to it
	Unbind();
}

void Texture::UploadToShader(const char* uniform, GLuint shaderID, GLint s) const
{
	glUseProgram(shaderID);
	glUniform1i(glGetUniformLocation(shaderID, uniform), s);
}
