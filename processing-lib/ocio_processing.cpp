#include "ocio_processing.h"

#include <OpenColorIO/OpenColorIO.h>
#include <GL/glew.h>
#include "glsl_shader.h"

const char* compute_aces_shader = "#version 440\n"
"layout (binding=0, rgba16f) uniform coherent image2D input_buffer;\n"
"layout (binding=1) uniform sampler3D aces_lut;\n"
"layout (local_size_x = 16, local_size_y = 16) in;\n"
"uniform mat3 color_mat;"
"vec4 texture3D(const sampler3D lut3d, vec3 xyz){return texture(lut3d, xyz);}\n"
"vec4 OCIODisplay(in vec4 inPixel, const sampler3D lut3d);\n"
"void main(){\n"
"ivec2 loadPos = ivec2(gl_GlobalInvocationID.xy);\n"
"vec4 col = imageLoad(input_buffer, loadPos);\n"
"col.xyz = col.xyz * color_mat;\n"
"col = OCIODisplay(col, aces_lut);\n"
"imageStore(input_buffer, loadPos, col);\n"
"}\n"
"\n";

struct ocio_impl{
	render::Shader _program;
	unsigned int _lut_texture;
};

using namespace OCIO_NAMESPACE::OCIO_VERSION_NS;

OCIO_processing::OCIO_processing(std::string colorspace_in, std::string colorspace_out)
{
	_imp = new ocio_impl;
	const int LUT3D_EDGE_SIZE = 33;
	ConstConfigRcPtr config = GetCurrentConfig();
	ConstProcessorRcPtr processor = config->getProcessor(colorspace_in.c_str(), colorspace_out.c_str());

	GpuShaderDesc shaderDesc;
	shaderDesc.setLanguage(GPU_LANGUAGE_GLSL_1_3);
	shaderDesc.setFunctionName("OCIODisplay");
	shaderDesc.setLut3DEdgeLen(LUT3D_EDGE_SIZE);
	const char* text = processor->getGpuShaderText(shaderDesc);

	float *lut3d = new float[LUT3D_EDGE_SIZE*LUT3D_EDGE_SIZE*LUT3D_EDGE_SIZE*3];
	processor->getGpuLut3D(lut3d, shaderDesc);

	glGenTextures(1, &_imp->_lut_texture);
	glBindTexture(GL_TEXTURE_3D, _imp->_lut_texture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB,
	             LUT3D_EDGE_SIZE, LUT3D_EDGE_SIZE, LUT3D_EDGE_SIZE,
	             0, GL_RGB,GL_FLOAT, lut3d);

	delete[] lut3d;

	std::string shader(compute_aces_shader);
	shader += "\n";
	shader += text;

	_imp->_program.init_from_string(shader);
}

OCIO_processing::~OCIO_processing()
{
	delete _imp;
}

std::vector<std::string> OCIO_processing::get_displays()
{
	std::vector<std::string> displays;
	ConstConfigRcPtr config = GetCurrentConfig();
	int num_displays = config->getNumDisplays();
	for (int i=0; i < num_displays; ++i){
		displays.push_back(config->getDisplay(i));
	}
	return displays;
}

void OCIO_processing::process(const Texture2D& tex, float* pre_color_matrix)
{
	float vals[9] = {1.0f,0.0f, 0.0f,
					0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 1.0f};
	GLuint loc = _imp->_program.getUniformLocation("color_mat");
	_imp->_program.bind();
	glUniformMatrix3fv(loc, 1, false, pre_color_matrix ? pre_color_matrix : vals);
	glBindImageTexture(0, tex.get_gltex(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D,_imp-> _lut_texture);
	glDispatchCompute(tex.width() / 16 + 1, tex.height() / 16 + 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	_imp->_program.unbind();
}
