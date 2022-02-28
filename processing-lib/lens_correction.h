#pragma once
#include <string>
#include <vector>
#include "../utils/texture2D.h"

class Lens_correction
{
	struct lens_corr_impl;
	float* _uv_distortion_table;
	float* _expo_table;
	int _width, _height, _starty;
	float _focus_distance, _aperture;
	bool _do_expo, _do_distort;
	lens_corr_impl* _imp;
public:
	Lens_correction(const std::string camera, const std::string lens, const int width, 
					const int height, float crop_factor, float aperture, float focus_distance, float focus_length,
					float sensor_ratio, float scale,
					bool do_expo, bool do_distort);
	~Lens_correction();

	bool valid();
	void apply_correction(Texture2D& texture);
	unsigned int expo_gl_texture();
	unsigned int uv_gl_texture();
	static std::vector<std::string> get_camera_models();
	static std::vector<std::string> get_lens_models(std::string camera_model);
	static void get_lens_min_max_focal(std::string lens, float& min, float &max);
	static float get_crop_factor(std::string camera);
};

