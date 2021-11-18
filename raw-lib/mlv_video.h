#pragma once

#include "video.h"
#include <string>

struct mlv_imp;

class Mlv_video : public Video_base
{
	mlv_imp* _imp = NULL;
public:
	Mlv_video(std::string filename);
	~Mlv_video();

	uint32_t black_level();
	uint32_t white_level();

	uint32_t frame_count();
	void free_buffer() override;

	float fps() override;
	uint16_t* get_raw_buffer(uint32_t frame, float idt_matrix[9], const Video_base::RawInfo& ri) override;
	uint16_t* get_raw_frame(uint32_t frame);
	std::string get_camera_name() override;
	std::string get_lens_name() override;

	uint32_t raw_resolution_x() override;
	uint32_t raw_resolution_y() override;
	float get_focal_length() override;
	float get_focal_dist() override;
	float get_aperture() override;
	uint32_t get_num_frames() override;
	float get_crop_factor() override;
	float get_final_crop_factor() override;
	uint32_t get_iso() override;
	uint32_t get_shutter() override;
	int get_pixel_binning_x() override;
    int get_pixel_binning_y() override;
	int get_sampling_factor_x() override;
    int get_sampling_factor_y() override;
	int get_bpp() override;

	virtual GLenum gl_pixel_type() {return GL_UNSIGNED_SHORT;}
	virtual GLenum gl_pixel_format() {return GL_RGB;}
};
