#pragma once

#include <string>
#include <vector>
#include "texture2D.h"

struct ocio_impl;

class OCIO_processor
{
	ocio_impl *_imp;
public:
	OCIO_processor(std::string colorspace_in, std::string colorspace_out);
	~OCIO_processor();

	void process(const Texture2D& tex, float* pre_color_matrix = 0L);

	static std::vector<std::string> get_displays();
};
