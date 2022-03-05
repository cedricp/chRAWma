#include "texture2D.h"
#include <stdio.h>
#include <stdlib.h>

Texture2D::Texture2D() {
	_texid = 0;
}

Texture2D::Texture2D(GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type,
					 GLint wrap_s, GLint wrap_t, GLint min_filter, GLint max_filter,const void *pixels) {
	_texid = 0;
	_init(level, internalformat, width, height, border, format, type, wrap_s, wrap_t, min_filter, max_filter, pixels);
}

Texture2D::~Texture2D() {
	if (_texid){
		glDeleteTextures(1, &_texid);
	}
}

void Texture2D::update_buffer(GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLint format, GLenum type, void* pixels)
{
	if (_texid){
		glBindTexture(GL_TEXTURE_2D, _texid);
		glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, format, type, pixels);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void* Texture2D::get_subdata_float_rgb(
 	GLint xoffset, GLint yoffset, GLint zoffset,
 	GLsizei width,GLsizei height,
 	GLint level, GLsizei depth)
{
	GLsizei bufSize = width * height * sizeof(float) * 3;
	void *pixels;
	pixels = (void*)malloc(bufSize);
	glGetTextureSubImage(_texid, level,
 	xoffset, yoffset, zoffset,
 	width, height, depth, GL_RGB, GL_FLOAT, bufSize, pixels);
	return pixels;
}

void Texture2D::_init(GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type,
					 GLint wrap_s, GLint wrap_t, GLint min_filter, GLint max_filter,const void *pixels)
{
	if (_texid){
		glDeleteTextures(1, &_texid);
	}
	_internal_format = internalformat;
	glGenTextures(1, &_texid);
	glBindTexture(GL_TEXTURE_2D, _texid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, max_filter);
	glTexImage2D(GL_TEXTURE_2D, level, internalformat, width, height, border, format, type, pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::clear(GLenum format, GLenum type, void* pixels)
{
	glClearTexImage(_texid, 0, format, type, pixels);
}

void* Texture2D::to_cpu_ram(GLint format, GLenum type)
{
	int pixelsize = 0;
	if (type == GL_FLOAT){
		pixelsize = sizeof(float);
	} else 	if (type == GL_UNSIGNED_SHORT || type == GL_HALF_FLOAT){
		pixelsize = sizeof(short);
	} else if (type == GL_UNSIGNED_BYTE){
		pixelsize = sizeof(char);
	} else if (type == GL_FLOAT || type == GL_UNSIGNED_INT || type == GL_INT){
		pixelsize = sizeof(short);
	}
	if (pixelsize == 0){
		printf("Texture2D::get_data : Pixel type not supported\n");
		return NULL;
	}
	int w = width();
	int h = height();
	
	void* data = malloc(w*h*pixelsize);
    glGetTexImage(GL_TEXTURE_2D, 0, format, type, data);
	return data;
}

void Texture2D::swap(Texture2D& s)
{
	if (_internal_format != s._internal_format){
		printf("Cannot texture swap, different types\n");
		return;
	}
	int temp_if = _internal_format;
	GLuint temp_id = _texid;

	_internal_format = s._internal_format;
	_texid = s._texid;

	s._internal_format = temp_if;
	s._texid = temp_id;
}

int Texture2D::width() const
{
	int w = 0;
	glBindTexture(GL_TEXTURE_2D, _texid);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glBindTexture(GL_TEXTURE_2D, 0);
	return w;
}

int Texture2D::height() const
{
	int h = 0;
	glBindTexture(GL_TEXTURE_2D, _texid);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
	glBindTexture(GL_TEXTURE_2D, 0);
	return h;
}
