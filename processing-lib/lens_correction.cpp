#include "lens_correction.h"
#include <lensfun/lensfun.h>

#include <GL/glew.h>
#include "../utils/glsl_shader.h"

const char* compute_distort_shader = "#version 440\n"
"layout (binding=0, rgba16f) uniform writeonly image2D output_buffer;\n"
"layout (binding=1, rg16f) uniform readonly image2D distort_buffer;\n"
"layout (binding=2) uniform sampler2D input_buffer;\n"
"layout (local_size_x = 16, local_size_y = 16) in;\n"
"void main(){\n"
"ivec2 sz = imageSize(output_buffer);\n"
"ivec2 loadPos = ivec2(gl_GlobalInvocationID.xy);\n"
"vec4 uv = imageLoad(distort_buffer, loadPos);\n"
"vec4 color = texture(input_buffer, uv.xy / sz);\n"
"\n"
"imageStore(output_buffer, loadPos, color);\n"
"}\n"
"\n";

const char* compute_copy_shader = "#version 440\n"
"layout (binding=0, rgba16f) uniform writeonly image2D output_buffer;\n"
"layout (binding=1, rgba16f) uniform readonly image2D input_buffer;\n"
"layout (binding=2, r16f) uniform readonly image2D expo_correction;\n"
"layout (local_size_x = 16, local_size_y = 16) in;\n"
"void main(){\n"
"ivec2 loadPos = ivec2(gl_GlobalInvocationID.xy);\n"
"vec4 color = imageLoad(input_buffer, loadPos);\n"
"vec4 expo = imageLoad(expo_correction, loadPos);\n"
"imageStore(output_buffer, loadPos, color * expo.x);\n"
"}\n"
"\n";

class lfdb {
	lfDatabase _ldb;
public:
	lfdb(){
		_ldb.Load();
	}
	lfDatabase& get(){
		return _ldb;
	}
};

static lfdb _lfdb;

struct Lens_correction::lens_corr_impl {
	render::Shader shader_distort;
	render::Shader shader_copy;
	GLuint uv_tex, out_tex, expo_tex;
};

Lens_correction::Lens_correction(const std::string camera, const std::string lens,
								 const int width, const int height,
								 float aperture, float focus_distance, float focus_length) : _width(width), _height(height),
								 _aperture(aperture), _focus_distance(focus_distance)
{
	_imp = new lens_corr_impl;

	_uv_distortion_table = NULL;
	_expo_table = NULL;
	_imp->uv_tex = _imp->out_tex = _imp->expo_tex = 0;

	const lfCamera *lfcam = NULL;
	const lfLens *lflens = NULL;

	const lfCamera ** cameras = _lfdb.get().FindCamerasExt(NULL, camera.c_str());
	if (!cameras){
		return;
	}
	lfcam = cameras[0];
    const lfLens **lenses = _lfdb.get().FindLenses (lfcam, NULL, lens.c_str());
    if (!lenses){
    	return;
    }
	lflens = lenses[0];

	_uv_distortion_table = new float[width*height*2];
	_expo_table = new float[width*height];

	for (int i = 0; i < width*height; ++i){
		_expo_table[i] = 1.0f;
	}

	lfModifier *mod = new lfModifier(lflens, focus_length, lfcam->CropFactor, width, height, LF_PF_F32, false);
	mod->EnableDistortionCorrection();
	mod->EnableVignettingCorrection(_aperture, _focus_distance);
	mod->ApplyGeometryDistortion(0.0, 0.0, width, height, _uv_distortion_table);
	mod->ApplyColorModification(_expo_table, 0, 0, width, height, LF_CR_1(INTENSITY), width*sizeof(float));

	std::string shader = compute_distort_shader;
	std::string shader_copy = compute_copy_shader;
	_imp->shader_distort.init_from_string(shader);
	_imp->shader_copy.init_from_string(shader_copy);

	glGenTextures(1, &_imp->expo_tex);
	glBindTexture(GL_TEXTURE_2D, _imp->expo_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, _width, _height, 0, GL_RED, GL_FLOAT, _expo_table);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &_imp->uv_tex);
	glBindTexture(GL_TEXTURE_2D, _imp->uv_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _width, _height, 0, GL_RG, GL_FLOAT, _uv_distortion_table);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &_imp->out_tex);
	glBindTexture(GL_TEXTURE_2D, _imp->out_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _width, _height, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	delete mod;
	delete[] _uv_distortion_table;
	delete[] _expo_table;
}

Lens_correction::~Lens_correction()
{
	glDeleteTextures(1, &_imp->uv_tex);
	glDeleteTextures(1, &_imp->out_tex);
	delete _imp;
}

void Lens_correction::apply_correction(unsigned int tex)
{
	_imp->shader_distort.bind();
	glBindImageTexture(0, _imp->out_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glBindImageTexture(1, _imp->uv_tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16F);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex);
	glDispatchCompute(_width / 16 + 1, _height / 16 + 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	_imp->shader_distort.unbind();

	_imp->shader_copy.bind();
	glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glBindImageTexture(1, _imp->out_tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
	glBindImageTexture(2, _imp->expo_tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16F);
	glDispatchCompute(_width / 16 + 1, _height / 16 + 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	_imp->shader_copy.unbind();

}
