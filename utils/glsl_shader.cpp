// ==========
// shader.cpp
// ==========

#include "glsl_shader.h"
#include <GL/glew.h>

#include <iostream>
#include <fstream>

using namespace render;

Shader::Shader()
{
	_shaders[0] = _shaders[1] = 0;
	_program  = 0;
}

Shader::Shader(std::string vs, std::string fs, bool isfile)
{
	_shaders[0] = _shaders[1] = 0;
	_program  = 0;
	if (isfile){
		init(vs, fs);
	} else {
		_shaders[0] = createShader(vs, GL_VERTEX_SHADER);
		_shaders[1] = createShader(fs, GL_FRAGMENT_SHADER);
	}
}

Shader::Shader(std::string cs, bool isfile)
{
	_shaders[0] = _shaders[1] = 0;
	_program  = 0;

	_program = glCreateProgram();
	if(isfile){
		_shaders[0] = createShader(loadShader(cs), GL_COMPUTE_SHADER);
	} else {
		_shaders[0] = createShader(cs, GL_COMPUTE_SHADER);
	}
	_shaders[1] = 0;

	linkProgram();
}

Shader::~Shader()
{
	if (_program == 0){
		return;
	}
	glDetachShader(_program, _shaders[0]);
	glDeleteShader(_shaders[0]);

	if (_shaders[1]){
		glDetachShader(_program, _shaders[1]);
		glDeleteShader(_shaders[1]);
	}
	glDeleteProgram(_program);
}

void Shader::init(std::string& fileName_vs, std::string& fileName_fs)
{
	_program = glCreateProgram();
	_shaders[0] = createShader(loadShader(fileName_vs), GL_VERTEX_SHADER);
	_shaders[1] = createShader(loadShader(fileName_fs), GL_FRAGMENT_SHADER);

	linkProgram();
}

void Shader::init_from_string(std::string& cs)
{
	_program = glCreateProgram();
	_shaders[0] = createShader(cs, GL_COMPUTE_SHADER);
	_shaders[1] = 0;

	linkProgram();
}

GLint Shader::getUniformLocation(std::string param)
{
	return glGetUniformLocation(_program, param.c_str());
}

void Shader::setUniform1f(std::string param, float val)
{
	glUniform1i(getUniformLocation(param), val);
}

void Shader::setUniform1i(std::string param, int val)
{
	glUniform1i(getUniformLocation(param), val);
}

void Shader::bind()
{
	glUseProgram(_program);
}

void Shader::unbind()
{
	glUseProgram(0);
}

std::string Shader::loadShader(const std::string& fileName)
{
	std::ifstream file;

	file.open((fileName).c_str());

	std::string output;
	std::string line;

	if(file.is_open())
	{
		while(file.good())
		{
			getline(file, line);
			output.append(line + "\n");
		}
	}
	else
		std::cerr << "[r] Unable to load shader: " << fileName << std::endl;

	return output;
}

void Shader::dispatchCompute(int x, int y, int z)
{
	glDispatchCompute(x, y, z);
}

void Shader::enableMemoryBarrier()
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

GLuint Shader::createShader(const std::string& text, unsigned int type)
{
	GLuint shader = glCreateShader(type);

	if(shader == 0)
		std::cerr << "[r] Error creating shader: " << type << std::endl;

	const GLchar* p[1];
	p[0] = text.c_str();
	GLint lengths[1];
	lengths[0] = text.length();

	glShaderSource(shader, 1, p, lengths);
	glCompileShader(shader);

	GLint isCompiled = 0;
	GLchar error[1024] = { 0 };

	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE)
	{
		glGetShaderInfoLog(shader, sizeof(error), NULL, error);
		std::cerr << "[r] Error compiling shader: " << error <<  std::endl;
	}

	return shader;
}

void Shader::linkProgram()
{
	glAttachShader(_program, _shaders[0]);
	if (_shaders[1]) glAttachShader(_program, _shaders[1]);
	glLinkProgram(_program);

	GLint isLinked = 0;
	GLchar error[1024] = { 0 };

	glGetProgramiv(_program, GL_LINK_STATUS, &isLinked);

	if (isLinked == GL_FALSE)
	{
		glGetProgramInfoLog(_program, sizeof(error), NULL, error);
		std::cerr << "[r] Error linking shaders:" << error << std::endl;
	}

	glValidateProgram(_program);

	glGetProgramiv(_program, GL_LINK_STATUS, &isLinked);

	if (isLinked == GL_FALSE)
	{
		glGetProgramInfoLog(_program, sizeof(error), NULL, error);
		std::cerr << "[r] Invalid shader program:" << error << std::endl;
	}
}
