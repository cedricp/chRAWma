#include "blur_processor.h"

#include <GL/glew.h>
#include "glsl_shader.h"

struct blur_impl{
	render::Shader _program;
    GLint _radius_uniform;
};

const char* compute_blur_shader = "#version 440\n"
"layout (binding=0, rgba16f) uniform writeonly image2D output_buffer;\n"
"layout (binding=1) uniform sampler2D input_buffer;\n"
"layout (local_size_x = 16, local_size_y = 16) in;\n"
"uniform vec2 radius;\n"
"const float PI    = 3.141592653589793;\n"
"const float sigma = ((radius.x + radius.y) - 1.0) / 6.0;\n"
"vec4 GaussianBlurHV(vec2 uv)\n"
"{\n"
"   ivec2 sz = imageSize(output_buffer);\n"
"	float twoSigmaSqu = 2 * sigma * sigma;\n"
"	float allWeight = 0.0f;\n"
"	vec4 col = vec4(0.0f, 0.0f, 0.0f, 0.0f);\n"
"	for (float ix = -radius.x; ix <= (radius.x); ix++){\n"
"		for (float iy = -radius.y; iy <= (radius.y); iy++){\n"
"           if (ix > radius.x) ix = radius.x;\n"
"           if (iy > radius.y) iy = radius.y;\n"
"			float weight   = 1.0f / (PI * twoSigmaSqu) * exp(-(ix * ix + iy * iy) / twoSigmaSqu);\n"
"			vec2 offset    = vec2(1.0f / sz.x * ix, 1.0f / sz.y * iy);\n"
"			vec2 uv_offset = uv + offset;\n"
"			uv_offset.x    = clamp(uv_offset.x, 0.0f, 1.0f);\n"
"			uv_offset.y    = clamp(uv_offset.y, 0.0f, 1.0f);\n"
"			col += texture2D(input_buffer, uv_offset) * weight;\n"
"			allWeight += weight;\n"
"		}\n"
"	}\n"
"   col = col / allWeight;\n"
"	return col;\n"
"}\n"
"void main(){\n"
"   ivec2 sz = imageSize(output_buffer);\n"
"   ivec2 loadPos = ivec2(gl_GlobalInvocationID.xy);\n"
"   vec2 uvPos = vec2(float(loadPos.x) / float(sz.x), float(loadPos.y) / float(sz.y));"
"   vec4 col = GaussianBlurHV(uvPos);\n"
"   imageStore(output_buffer, loadPos, col);\n"
"}\n";


Blur_processor::Blur_processor()
{
    _imp = new blur_impl;
    std::string shader = compute_blur_shader;
    _imp->_program.init_from_string(shader);
    _imp->_radius_uniform = _imp->_program.getUniformLocation("radius");
}

Blur_processor::~Blur_processor()
{
    delete _imp;
}

void Blur_processor::process(const Texture2D& in_tex, Texture2D& out_tex, float blur_size)
{
	int tw = (out_tex.width() + 15) / 16;
	int th = (out_tex.height() + 15) / 16;
	_imp->_program.bind();
	glBindImageTexture(0, out_tex.gl_texture(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, in_tex.gl_texture());
    glUniform2f(_imp->_radius_uniform, blur_size, 1.0f);
	glDispatchCompute(tw, th, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	_imp->_program.unbind();
}