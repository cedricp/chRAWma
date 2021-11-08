#pragma once

#include <string>

class Lookup_table{
	float *m_cube = NULL;
	bool m_is3d;
	float m_domain_min[3];
	float m_domain_max[3];
	unsigned int m_dimension = 0;
public:
	Lookup_table(std::string filename);
	~Lookup_table();

	bool is3d(){return m_is3d;}
	float* get_data(){return m_cube;}
	int get_dimension(){return m_dimension;}
};
