#include "waveform_monitor.h"
#include <GL/glew.h>

std::string wf_compute_src = "#version 440\n \
\n \
layout (binding = 0, r32ui) uniform coherent uimage2D destTex;\n \
layout (binding = 1, rgba16f) uniform readonly image2D sourceTex;\n \
layout (local_size_x = 64, local_size_y = 4, local_size_z = 1) in;\n \
\n \
uniform float scale_y;\n \
uniform int parade;\n \
uniform int srgb;\n \
\n \
void main() \n \
{\n \
	ivec2 imgsize = imageSize(sourceTex);\n \
	ivec2 scopesize = imageSize(destTex);\n \
	scopesize.x /= 3;\n \
	ivec2 loadPos = ivec2(gl_GlobalInvocationID.xy);\n \
	\n \
	int storeX = int(float(loadPos.x) / float(imgsize.x) * float(scopesize.x));\n \
	\n \
	vec4 col = imageLoad(sourceTex, loadPos) * scale_y;\n \
	\n \
	if (srgb > 0){\n \
		col.x = pow(col.x, 0.4545f);\n \
		col.y = pow(col.y, 0.4545f);\n \
		col.z = pow(col.z, 0.4545f);\n \
	}\n \
	\n \
	if (parade == 0){\n \
		memoryBarrier();\n \
\n \
		ivec2 storePos = ivec2(storeX, 0);\n \
		storePos.y = int( (1.0f -(col.x > 1.0f ? 1.0f : col.x)) * float(scopesize.y));\n \
		imageAtomicAdd(destTex, storePos, 1);\n \
		\n \
		storePos.x += scopesize.x;\n \
		storePos.y = int( (1.0f -(col.y > 1.0f ? 1.0f : col.y)) * float(scopesize.y));\n \
		imageAtomicAdd(destTex, storePos, 1);\n \
		\n \
		storePos.x += scopesize.x;\n \
		storePos.y = int( (1.0f -(col.z > 1.0f ? 1.0f : col.z)) * float(scopesize.y));\n \
		imageAtomicAdd(destTex, storePos, 1);\n \
	} else {\n \
		const int ww = scopesize.x + (scopesize.x/3);\n \
		ivec2 storePos = ivec2(storeX / 3, int( (1.0f -(col.x > 1.0f ? 1.0f : col.x)) * float(scopesize.y)) );\n \
\n \
		memoryBarrier();\n \
\n \
		// Red\n \
		imageAtomicAdd(destTex, storePos, 1);\n \
		\n \
		// Green\n \
		storePos = ivec2(storeX / 3  + ww, int( (1.0f -(col.y > 1.0f ? 1.0f : col.y)) * float(scopesize.y)) );\n \
		imageAtomicAdd(destTex, storePos, 1);\n \
		\n \
		// Blue\n \
		storePos = ivec2(storeX / 3 + ww + ww, int( (1.0f -(col.z > 1.0f ? 1.0f : col.z)) * float(scopesize.y)) );\n \
		imageAtomicAdd(destTex, storePos, 1);\n \
	}\n \
}";

std::string wf_comput_mix_src = "#version 440\n \
layout (binding = 0, r32ui) uniform readonly uimage2D source;\n \
layout (binding = 1, rgba16f) uniform writeonly image2D dest;\n \
layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;\n \
\n \
uniform float spot_intensity;\n \
uniform float scale_y;\n \
\n \
void main()\n \
{\n \
	ivec2 size = imageSize(source);\n \
	size.x /= 3;\n \
	ivec2 center = size / 2;\n \
	ivec2 loadPos = ivec2(gl_GlobalInvocationID.xy);\n \
	uvec4 colr = imageLoad(source, loadPos);\n \
	loadPos.x += size.x;\n \
	uvec4 colg = imageLoad(source, loadPos);\n \
	loadPos.x += size.x;\n \
	uvec4 colb = imageLoad(source, loadPos);\n \
	\n \
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);\n \
	\n \
	vec4 finalColor =vec4(0.,0.,0.,1.);\n \
	\n \
	\n \
	if (size.y - storePos.y == int(float(size.y) * 0.5f * scale_y)){\n \
		finalColor.x += 0.1f;\n \
		finalColor.y += 0.1f;\n \
		finalColor.z += 0.1f;\n \
	}\n \
	\n \
	finalColor.x += float(colr.x) / 4096.0;\n \
	finalColor.y += float(colg.x) / 4096.0;\n \
	finalColor.z += float(colb.x) / 4096.0;\n \
	\n \
	finalColor *= spot_intensity;\n \
	\n \
	\n \
	finalColor.x = pow(finalColor.x, 0.4545f);\n \
	finalColor.y = pow(finalColor.y, 0.4545f);\n \
	finalColor.z = pow(finalColor.z, 0.4545f);\n \
	\n \
	imageStore(dest, storePos, finalColor);\n \
}";

waveformMonitor::waveformMonitor(int w, int h) : _height(h), _width(w),
												 _compute_shader(wf_compute_src),
												 _compute_shader_mix(wf_comput_mix_src)
{
	_inh = _inw = 0;
	prepareOutput_textures();

	_wf_scale_loc = _compute_shader.getUniformLocation("scale_y");
	_wf_srgb_loc = _compute_shader.getUniformLocation("srgb");
	_wf_parade_loc = _compute_shader.getUniformLocation("parade");

	_mix_scale_loc = _compute_shader_mix.getUniformLocation("scale_y");
	_mix_spot_loc = _compute_shader_mix.getUniformLocation("spot_intensity");
}

waveformMonitor::~waveformMonitor()
{
}

const Texture2D& waveformMonitor::compute(const Texture2D& tex)
{
	_inw = tex.width();
	_inh = tex.height();

	if (!_inw || !_inh){
		return _output_texture;
	}

	clearTextures();

	_compute_shader.bind();
	glUniform1f(_wf_scale_loc, _wf_scale);
	glUniform1i(_wf_srgb_loc, _wf_srgb);
	glUniform1i(_wf_parade_loc, _wf_parade);
	glBindImageTexture (0, _intermediate_texture.get_gltex(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
	glBindImageTexture (1, tex.get_gltex(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
	glDispatchCompute(_inw / 64, _inh / 4, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	_compute_shader.unbind();

	_compute_shader_mix.bind();
	glBindImageTexture (0, _intermediate_texture.get_gltex(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
	glBindImageTexture (1, _output_texture.get_gltex(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glUniform1f(_mix_spot_loc, _wf_spot_intensity);
	glUniform1f(_mix_scale_loc, _wf_scale);
	glDispatchCompute(_width / 16, _height / 16, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	_compute_shader_mix.unbind();

	return _output_texture;
}

void waveformMonitor::prepareOutput_textures()
{
	_intermediate_texture.init(GL_RED_INTEGER, GL_UNSIGNED_INT, _width * 3, _height);
	_output_texture.init(GL_RGBA, GL_HALF_FLOAT, _width, _height);
}

void waveformMonitor::clearTextures()
{
	_intermediate_texture.clear(GL_RED_INTEGER, GL_UNSIGNED_INT);
	_output_texture.clear(GL_RGBA, GL_HALF_FLOAT);
}
