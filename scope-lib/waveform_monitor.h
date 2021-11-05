#pragma once

#include "../utils/glsl_shader.h"

class waveformMonitor
{
	unsigned int _input_texture;
	unsigned int _output_texture_integer;
	unsigned int _output_texture;
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
	float _wf_spot_intensity = 1.0f;

	unsigned int _mix_spot_loc;
	unsigned int _mix_scale_loc;

	void deleteTextures();
	void prepareOutput_textures();
	void clearTextures();

public:
	waveformMonitor(int w, int h);
	~waveformMonitor();
	unsigned int compute(unsigned int input_tex);

};
