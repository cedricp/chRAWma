#include <inttypes.h>
#include "3dlut.h"

Lookup_table::Lookup_table(std::string filename) {
	uint32_t lut_size = 0;
	uint32_t i = 0;

	this->m_dimension = 1;
	this->m_cube = NULL;

	char line[250], title[128];

	float r, g, b;
	float inMin, inMax;

	bool valid = true;

	FILE *fp;

	fp = fopen(filename.c_str(), "r");

	if (fp == nullptr) {
		return;
	}

	for (int j = 0; j < 3; j++) {
		this->m_domain_min[j] = 0.0;
		this->m_domain_max[j] = 1.0;
	}

	while (fgets(line, 250, fp) != NULL) {
		if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
			continue;
		} else if (sscanf(line, "TITLE%*[ \t]%[^\n]", title) == 1) {
			continue;
		} else if (sscanf(line, "LUT_1D_SIZE%*[ \t]%hu%*[^\n]",
				&this->m_dimension) == 1) {
			lut_size = this->m_dimension * 3;
			m_is3d = false;
			m_cube = new float[lut_size];
			;
			continue;
		} else if (sscanf(line, "LUT_3D_SIZE%*[ \t]%hu%*[^\n]",
				&this->m_dimension) == 1) {
			lut_size = (uint32_t) this->m_dimension
					* (uint32_t) this->m_dimension
					* (uint32_t) this->m_dimension * 3;
			m_is3d = true;
			m_cube = new float[lut_size];
			continue;
		} else if (sscanf(line, "%f%*[ \t]%f%*[ \t]%f%*[^\n]", &r, &g, &b)
				== 3) {
			if (!lut_size || i >= lut_size) {
				valid = false;
				break;
			}
			this->m_cube[i++] = r;
			this->m_cube[i++] = g;
			this->m_cube[i++] = b;
		} else if (sscanf(line, "DOMAIN_MIN%*[ \t]%f%*[ \t]%f%*[ \t]%f%*[^\n]",
				&this->m_domain_min[0], &this->m_domain_min[1],
				&this->m_domain_min[2]) == 3) {
			continue;
		} else if (sscanf(line, "DOMAIN_MAX%*[ \t]%f%*[ \t]%f%*[ \t]%f%*[^\n]",
				&this->m_domain_max[0], &this->m_domain_max[1],
				&this->m_domain_max[2]) == 3) {
			continue;
		} else if (sscanf(line, "LUT_1D_INPUT_RANGE%*[ \t]%f%*[ \t]%f%*[^\n]",
				&inMin, &inMax) == 2) {
			continue;
		} else if (sscanf(line, "LUT_3D_INPUT_RANGE%*[ \t]%f%*[ \t]%f%*[^\n]",
				&inMin, &inMax) == 2) {
			continue;
		} else {
			valid = false;
			break;
		}
	}

	if (i < lut_size || valid == false) {
		if (this->m_cube) {
			delete[] this->m_cube;
		}
		this->m_cube = NULL;
	}

	fclose(fp);
}

Lookup_table::~Lookup_table() {
	if (m_cube) {
		delete[] m_cube;
	}
}
