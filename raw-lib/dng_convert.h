#pragma once
#include "mlv-lib/video_mlv.h"
#include <string>

class Dng_processor
{
public:
	struct Idt{
		float matrix[9];
	};
	Dng_processor();
	~Dng_processor();

	uint16_t* get_aces(uint8_t* buffer, size_t buffersize);
	uint16_t* get_aces_from_file(std::string dng_filename);
	Idt get_idt_matrix();
	void free_buffer();
	int width(){return _w;}
	int height(){return _h;}
	void idt(float idt[9]){for(int i = 0; i < 9; ++i){idt[i] = _idt.matrix[i];} }
	void set_interpolation(int i){_interpolation = i;}
private:
	struct dngc_impl;
	dngc_impl* _imp;
	unsigned short *_buffer;
	Idt _idt;
	int _w, _h;
	int _interpolation;
};
