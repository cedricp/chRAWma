#pragma once
#include <string>
#include <vector>
#include "texture2D.h"

struct blur_impl;

class Blur_processor
{
	blur_impl *_imp;
public:
	Blur_processor();
	~Blur_processor();

	void process(const Texture2D& in_tex, Texture2D& out_tex, float raduis, float hv_ratio);
};
