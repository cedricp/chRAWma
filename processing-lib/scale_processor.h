#pragma once
#include <string>
#include <vector>
#include "texture2D.h"

struct scale_impl;

class Scale_processor
{
	scale_impl *_imp;
public:
	Scale_processor();
	~Scale_processor();

	void process(const Texture2D& in_tex, Texture2D& out_tex);
};
