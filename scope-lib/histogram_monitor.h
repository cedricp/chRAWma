#pragma once

#include "../utils/glsl_shader.h"
#include "../utils/texture2D.h"

class histogramMonitor
{
	uint32_t _width, _height;
	int _inw, _inh;
	render::Shader _compute_shader;
	render::Shader _compute_shader_mix;

	GLuint _histogram_buffer_id;

    unsigned int _histogram_width_loc;
    unsigned int _histogram_scale_loc;

	TextureRGBA16F _output_texture;

    uint32_t *_cpu_buffer;

	void deleteTextures();
	void prepareOutput_textures();
	void clearTextures();

public:
	histogramMonitor(int w, int h);
	~histogramMonitor();
	const Texture2D& compute(const Texture2D& input_tex);
};
