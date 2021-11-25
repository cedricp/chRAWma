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

	uint16_t* raw_buffer(uint32_t frame, float idt_matrix[9], const Video_base::RawInfo& ri) override;

	uint32_t black_level();
	uint32_t white_level();

	void free_buffer() override;
	float fps() override;
	uint32_t frame_count() override;

	uint32_t raw_resolution_x() override;
	uint32_t raw_resolution_y() override;
	std::string camera_name() override;
	std::string lens_name() override;
	std::string lens_name_by_id() override;
	float focal_length() override;
	float focal_dist() override;
	float aperture() override;
	float crop_factor() override;
	float final_crop_factor() override;
	uint32_t iso() override;
	uint32_t shutter_speed() override;
	int pixel_binning_x() override;
    int pixel_binning_y() override;
	int sampling_factor_x() override;
    int sampling_factor_y() override;
	int bpp() override;
	void sensor_resolulion(int& x, int& y) override;

	virtual GLenum gl_pixel_type() {return GL_UNSIGNED_SHORT;}
	virtual GLenum gl_pixel_format() {return GL_RGB;}
};
