#pragma once
#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include <cerrno>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>

std::string get_file_contents(const char* filename);

class Shader
{
public:
	GLuint ID;
	Shader();
	Shader(const char* vertexFile, const char* fragmentFile);

	void LoadFromFile(const char* vertexFile, const char* fragmentFile);
	void Activate();
	void Delete();

private:
	void compileErrors(unsigned int shader, const char* type);
};

#endif