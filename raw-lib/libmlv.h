#pragma once

#include <string>

struct mlv_imp;

class Mlv_video
{
	mlv_imp* _imp = NULL;
public:
	Mlv_video(std::string filename);
	~Mlv_video();

	uint32_t resolution_x();
	uint32_t resolution_y();

	uint32_t black_level();
	uint32_t white_level();

	float fps();
	uint32_t frame_count();

	uint16_t* get_raw_frame(uint32_t frame);
	unsigned short* get_raw_processed_buffer(uint32_t frame, float idt_matrix[9]);
	void free_buffer();

	std::string get_camera_name();
	std::string get_lens_name();

	float get_focal_length();
	float get_focal_dist();
	float get_aperture();
};
