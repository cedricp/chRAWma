#include "lens_correction.h"
#include <lensfun/lensfun.h>

#include "../utils/texture2D.h"
#include "../utils/glsl_shader.h"

const char* compute_distort_shader = "#version 440\n"
"layout (binding=0, rgba16f) uniform writeonly image2D output_buffer;\n"
"layout (binding=1, rg16f) uniform readonly image2D distort_buffer;\n"
"layout (binding=2) uniform sampler2D input_buffer;\n"
"layout (binding=3, r16f) uniform readonly image2D expo_correction;\n"
"layout (local_size_x = 16, local_size_y = 16) in;\n"
"uniform bool do_expo;\n"
"uniform bool do_uv;\n"
"uniform int starty;"
"void main(){\n"
"ivec2 sz = imageSize(output_buffer);\n"
"ivec2 loadPos = ivec2(gl_GlobalInvocationID.xy);\n"
"vec4 uv;\n"
"if (do_uv){uv = imageLoad(distort_buffer, loadPos);uv.y -= starty;}\n"
"else uv = vec4(float(loadPos.x), float(loadPos.y), 0.,0.);\n"
"vec4 color = texture(input_buffer, uv.xy / sz);\n"
"if (do_expo){color *= imageLoad(expo_correction, loadPos).x;}\n"
"imageStore(output_buffer, loadPos, color);\n"
"}\n"
"\n";

const char* compute_copy_shader = "#version 440\n"
"layout (binding=0, rgba16f) uniform writeonly image2D output_buffer;\n"
"layout (binding=1, rgba16f) uniform readonly image2D input_buffer;\n"
"layout (local_size_x = 16, local_size_y = 16) in;\n"
"void main(){\n"
"ivec2 loadPos = ivec2(gl_GlobalInvocationID.xy);\n"
"vec4 color = imageLoad(input_buffer, loadPos);\n"
"imageStore(output_buffer, loadPos, color);\n"
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
	TextureRGBA16F out_tex;
	TextureRG16F uv_tex;
	TextureR16F expo_tex;
	bool valid;
};

Lens_correction::Lens_correction(const std::string camera, const std::string lens,
								 const int width, const int height, float crop_factor,
								 float aperture, float focus_distance, float focus_length, float sensor_ratio, float scale, bool do_expo, bool do_distort) : _width(width), _height(height),
								 _aperture(aperture), _focus_distance(focus_distance), _do_expo(do_expo), _do_distort(do_distort)
{
	_imp = new lens_corr_impl;
	_imp->valid = false;

	_uv_distortion_table = NULL;
	_expo_table = NULL;

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

	int h = (float)width / sensor_ratio;

	if (do_distort){
		_uv_distortion_table = new float[width*height*2];
	}
	if (do_expo){
		 _expo_table = new float[width*height];

		for (int i = 0; i < width*height; ++i){
			_expo_table[i] = 1.f;
		}
	}

	_starty = (h - height) / 2;

	float crop = crop_factor != 0 ? crop_factor : lfcam->CropFactor;

	lfModifier *mod = new lfModifier(lflens, focus_length, crop, width, h, LF_PF_F32, false);
	if (do_expo){
		mod->EnableVignettingCorrection(_aperture, _focus_distance);
	}
	if (do_distort){
		mod->EnableDistortionCorrection();
	}
	if (scale != 1.0f){
		mod->EnableScaling(scale);
	}
	if (do_expo){
		mod->ApplyColorModification(_expo_table, 0.0, _starty, width, height, LF_CR_1(INTENSITY), width*sizeof(float));
	}
	if (do_distort){
		mod->ApplyGeometryDistortion(0.0f, _starty, width, height, _uv_distortion_table);
	}

	std::string shader = compute_distort_shader;
	std::string shader_copy = compute_copy_shader;
	_imp->shader_distort.init_from_string(shader);

	if(do_expo){
		_imp->expo_tex.init(GL_RED, GL_FLOAT, _width, _height, _expo_table);
	}
	if (do_distort){
		_imp->uv_tex.init(GL_RG, GL_FLOAT, _width, _height, _uv_distortion_table);
	}
	_imp->out_tex.init(GL_RGBA, GL_FLOAT, _width, _height);

	delete mod;
	if (do_distort)	delete[] _uv_distortion_table;
	if (do_expo)	delete[] _expo_table;

	_imp->valid = true;
}

Lens_correction::~Lens_correction()
{
	delete _imp;
}

bool Lens_correction::valid()
{
	return _imp->valid;
 }

void Lens_correction::apply_correction(Texture2D& tex)
{
	_imp->shader_distort.bind();
	glUniform1i(_imp->shader_distort.getUniformLocation("do_expo"), _do_expo);
	glUniform1i(_imp->shader_distort.getUniformLocation("do_uv"), _do_distort);
	glUniform1i(_imp->shader_distort.getUniformLocation("starty"), _starty);	

	_imp->out_tex.bindImageTexture(0, GL_WRITE_ONLY);
	if (_do_distort) _imp->uv_tex.bindImageTexture(1, GL_READ_ONLY);
	tex.bindTexture(2);
	if (_do_expo) _imp->expo_tex.bindImageTexture(3, GL_READ_ONLY);

	_imp->shader_distort.dispatchCompute((_width + 15) / 16, (_height + 15) / 16);
	_imp->shader_distort.enableMemoryBarrier();

	_imp->shader_distort.unbind();

	tex.swap(_imp->out_tex);
}

unsigned int Lens_correction::expo_gl_texture()
{
	return _imp->expo_tex.gl_texture();
}

unsigned int Lens_correction::uv_gl_texture()
{
return _imp->uv_tex.gl_texture();
}

std::vector<std::string> Lens_correction::get_camera_models()
{
	std::vector<std::string> camera_list;
	const lfCamera ** cameras = _lfdb.get().FindCamerasExt("Canon", NULL);
	if (!cameras){
	 return camera_list;
	}
	while(*cameras){
		camera_list.push_back((*cameras)->Model);
		cameras++;
	}
	return camera_list;
}

std::vector<std::string> Lens_correction::get_lens_models(std::string camera_model)
{
	std::vector<std::string> lens_list;
	const lfCamera ** cameras = _lfdb.get().FindCamerasExt(NULL, camera_model.c_str());
	
	if (!cameras){
		return lens_list;
	}

	const lfLens ** lenses = _lfdb.get().FindLenses(cameras[0], NULL, NULL);
	if (!lenses){
		return lens_list;
	}

	while(*lenses){
		lens_list.push_back((*lenses)->Model);
		lenses++;
	}

	return lens_list;
}

void Lens_correction::get_lens_min_max_focal(std::string lens, float& min, float &max)
{
	const lfLens ** lenses = _lfdb.get().FindLenses(NULL, NULL, lens.c_str());
	if (!lenses){
		return;
	}

	min = lenses[0]->MinFocal;
	max = lenses[0]->MaxFocal;
}

float Lens_correction::get_crop_factor(std::string camera)
{
	std::vector<std::string> lens_list;
	const lfCamera ** cameras = _lfdb.get().FindCamerasExt(NULL, camera.c_str());
	
	if (!cameras){
		return 1.0f;

	}

	return cameras[0]->CropFactor;
}