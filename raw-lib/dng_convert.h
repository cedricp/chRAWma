#pragma once
#include "mlv-lib/video_mlv.h"

class Dng_converter
{
public:
	struct Idt{
		float matrix[9];
	};
	Dng_converter(mlvObject_t* mlv_object);
	~Dng_converter();

	uint16_t* get_buffer(uint32_t frame);
	Idt get_idt_matrix();
	uint8_t* get_dng_buffer(uint32_t frame, uint32_t& size);
	void free_buffer();
private:
	struct dngc_impl;
	dngc_impl* _imp;
	unsigned short *_buffer;
	Idt _idt;
};
