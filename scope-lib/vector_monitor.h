#pragma once

#include "../utils/glsl_shader.h"
#include "../utils/texture2D.h"

class vectorMonitor
{
	uint32_t _width, _height;
	int _inw, _inh;
	render::Shader _compute_shader;
	render::Shader _compute_shader_mix;
	float _spot_intensity = 2.0f;
    float _scale = 1.0f;

	unsigned int _mix_spot_loc;
	unsigned int _mix_scale_loc;
    unsigned int _mix_colorwheel_loc;


	TextureR32UI _intermediate_texture;
	TextureRGBA16F _output_texture;

	void deleteTextures();
	void prepareOutput_textures();
	void clearTextures();

public:
	vectorMonitor(int w, int h);
	~vectorMonitor();
	const Texture2D& compute(const Texture2D& input_tex);

	void set_intensity(float i){_spot_intensity = i;}
	void set_scale(float s){_scale = s;}
};
