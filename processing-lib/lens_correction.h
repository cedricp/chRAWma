#pragma once
#include <string>


class Lens_correction
{
	struct lens_corr_impl;
	float* _uv_distortion_table;
	float* _expo_table;
	int _width, _height;
	float _focus_distance, _aperture;
	lens_corr_impl* _imp;
public:
	Lens_correction(const std::string camera, const std::string lens, const int width,
					const int height, float aperture, float focus_distance, float focus_length);
	~Lens_correction();

	void apply_correction(unsigned int tex);
};
