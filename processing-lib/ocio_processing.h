#pragma once
#include <string>

struct ocio_impl;

class OCIO_processing
{
	ocio_impl *_imp;
public:
	OCIO_processing(std::string colorspace_in, std::string colorspace_out);
	~OCIO_processing();

	void process(unsigned int tex, int w, int h, float* pre_color_matrix = 0L);
};
