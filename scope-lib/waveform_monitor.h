#pragma once

#include "../utils/glsl_shader.h"
#include "../utils/texture2D.h"

class waveformMonitor
{
	uint32_t _width, _height;
	int _inw, _inh;
	render::Shader _compute_shader;
	render::Shader _compute_shader_mix;
	unsigned int _wf_scale_loc;
	unsigned int _wf_srgb_loc;
	unsigned int _wf_parade_loc;
	float _wf_scale = 1.0;
	int _wf_srgb = 0;
	int _wf_parade = 1;
	float _wf_spot_intensity = 2.0f;

	unsigned int _mix_spot_loc;
	unsigned int _mix_scale_loc;

	TextureR32UI _intermediate_texture;
	TextureRGBA16F _output_texture;

	void deleteTextures();
	void prepareOutput_textures();
	void clearTextures();

public:
	waveformMonitor(int w, int h);
	~waveformMonitor();
	const Texture2D& compute(const Texture2D& input_tex);

	void set_intensity(float i){_wf_spot_intensity = i;}
	void set_parade(bool p){_wf_parade = p;}
	void set_srgb(bool s){_wf_srgb = s;}
	void set_scale(float s){_wf_scale = s;}
};
