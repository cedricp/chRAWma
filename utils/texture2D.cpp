#include "texture2D.h"
#include <stdio.h>

Texture2D::Texture2D() {
	_texid = 0;
	_width = 0;
	_height = 0;
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

void Texture2D::_init(GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type,
					 GLint wrap_s, GLint wrap_t, GLint min_filter, GLint max_filter,const void *pixels)
{
	if (_texid){
		glDeleteTextures(1, &_texid);
	}
	_width = width;
	_height = height;
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

void Texture2D::swap(Texture2D& s)
{
	if (_internal_format != s._internal_format){
		printf("Cannot texture swap, different types\n");
		return;
	}
	int temp_if = _internal_format;
	int temp_w = _width;
	int temp_h = _height;
	GLuint temp_id = _texid;

	_internal_format = s._internal_format;
	_width = s._width;
	_height = s._height;
	_texid = s._texid;

	s._internal_format = temp_if;
	s._height = temp_h;
	s._width = temp_w;
	s._texid = temp_id;
}