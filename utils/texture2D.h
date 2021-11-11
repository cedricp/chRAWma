#pragma once

#include "GL/glew.h"

class Texture2D
{
	GLuint _texid;
	int _width, _height;
public:
	Texture2D();
	Texture2D(GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type,
						 GLint wrap_s, GLint wrap_t, GLint min_filter, GLint max_filter,const void *pixels);
	virtual ~Texture2D();

	void update_buffer(GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLint format, GLenum type, void* pixels);
	GLuint get_gltex() const {return _texid;}

	virtual void init(GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type,
						 GLint wrap_s, GLint wrap_t, GLint min_filter, GLint max_filter,const void *pixels);

	int width() const {return _width;}
	int height() const {return _height;}

	void clear(GLenum format = GL_RGB, GLenum type = GL_FLOAT, void* pixels = NULL);
};

class TextureRGBA16F : public Texture2D
{
public:
	TextureRGBA16F(GLenum format, GLenum type, GLsizei width, GLsizei height,  const void *pixels = NULL, GLint border = 0,
			 GLint wrap_s = GL_CLAMP_TO_EDGE, GLint wrap_t = GL_CLAMP_TO_EDGE, GLint min_filter = GL_LINEAR, GLint max_filter = GL_LINEAR) : Texture2D(0, GL_RGBA16F, width, height,
			 border, format, type, wrap_s, wrap_t, min_filter, max_filter, pixels)
	{

	}

	TextureRGBA16F() : Texture2D(){}

	virtual void init(GLenum format, GLenum type, GLsizei width, GLsizei height,  const void *pixels = NULL, GLint border = 0,
			 GLint wrap_s = GL_CLAMP_TO_EDGE, GLint wrap_t = GL_CLAMP_TO_EDGE, GLint min_filter = GL_LINEAR, GLint max_filter = GL_LINEAR)
	{
		Texture2D::init(0, GL_RGBA16F, width, height, border, format, type, wrap_s, wrap_t, min_filter, max_filter, pixels);
	}
};

class TextureRGB16F : public Texture2D
{
public:
	TextureRGB16F(GLenum format, GLenum type, GLsizei width, GLsizei height,  const void *pixels = NULL, GLint border = 0,
			 GLint wrap_s = GL_CLAMP_TO_EDGE, GLint wrap_t = GL_CLAMP_TO_EDGE, GLint min_filter = GL_LINEAR, GLint max_filter = GL_LINEAR) : Texture2D(0, GL_RGB16F, width, height,
			 border, format, type, wrap_s, wrap_t, min_filter, max_filter, pixels)
	{

	}
	TextureRGB16F() : Texture2D(){}

	virtual void init(GLenum format, GLenum type, GLsizei width, GLsizei height,  const void *pixels = NULL, GLint border = 0,
			 GLint wrap_s = GL_CLAMP_TO_EDGE, GLint wrap_t = GL_CLAMP_TO_EDGE, GLint min_filter = GL_LINEAR, GLint max_filter = GL_LINEAR)
	{
		Texture2D::init(0, GL_RGB16F, width, height, border, format, type, wrap_s, wrap_t, min_filter, max_filter, pixels);
	}
};

class TextureRG16F : public Texture2D
{
public:
	TextureRG16F(GLenum format, GLenum type, GLsizei width, GLsizei height,  const void *pixels = NULL, GLint border = 0,
			 GLint wrap_s = GL_CLAMP_TO_EDGE, GLint wrap_t = GL_CLAMP_TO_EDGE, GLint min_filter = GL_LINEAR, GLint max_filter = GL_LINEAR) : Texture2D(0, GL_RG16F, width, height,
			 border, format, type, wrap_s, wrap_t, min_filter, max_filter, pixels)
	{

	}
	TextureRG16F() : Texture2D(){}

	virtual void init(GLenum format, GLenum type, GLsizei width, GLsizei height,  const void *pixels = NULL, GLint border = 0,
			 GLint wrap_s = GL_CLAMP_TO_EDGE, GLint wrap_t = GL_CLAMP_TO_EDGE, GLint min_filter = GL_LINEAR, GLint max_filter = GL_LINEAR)
	{
		Texture2D::init(0, GL_RG16F, width, height, border, format, type, wrap_s, wrap_t, min_filter, max_filter, pixels);
	}
};

class TextureR16F : public Texture2D
{
public:
	TextureR16F(GLenum format, GLenum type, GLsizei width, GLsizei height,  const void *pixels = NULL, GLint border = 0,
			 GLint wrap_s = GL_CLAMP_TO_EDGE, GLint wrap_t = GL_CLAMP_TO_EDGE, GLint min_filter = GL_LINEAR, GLint max_filter = GL_LINEAR) : Texture2D(0, GL_R16F, width, height,
			 border, format, type, wrap_s, wrap_t, min_filter, max_filter, pixels)
	{

	}
	TextureR16F() : Texture2D(){}

	virtual void init(GLenum format, GLenum type, GLsizei width, GLsizei height,  const void *pixels = NULL, GLint border = 0,
			 GLint wrap_s = GL_CLAMP_TO_EDGE, GLint wrap_t = GL_CLAMP_TO_EDGE, GLint min_filter = GL_LINEAR, GLint max_filter = GL_LINEAR)
	{
		Texture2D::init(0, GL_R16F, width, height, border, format, type, wrap_s, wrap_t, min_filter, max_filter, pixels);
	}
};

class TextureR32UI : public Texture2D
{
public:
	TextureR32UI(GLenum format, GLenum type, GLsizei width, GLsizei height,  const void *pixels = NULL, GLint border = 0,
			 GLint wrap_s = GL_CLAMP_TO_EDGE, GLint wrap_t = GL_CLAMP_TO_EDGE, GLint min_filter = GL_LINEAR, GLint max_filter = GL_LINEAR) : Texture2D(0, GL_R32UI, width, height,
			 border, format, type, wrap_s, wrap_t, min_filter, max_filter, pixels)
	{

	}
	TextureR32UI() : Texture2D(){}

	virtual void init(GLenum format, GLenum type, GLsizei width, GLsizei height,  const void *pixels = NULL, GLint border = 0,
			 GLint wrap_s = GL_CLAMP_TO_EDGE, GLint wrap_t = GL_CLAMP_TO_EDGE, GLint min_filter = GL_LINEAR, GLint max_filter = GL_LINEAR)
	{
		Texture2D::init(0, GL_R32UI, width, height, border, format, type, wrap_s, wrap_t, min_filter, max_filter, pixels);
	}
};
