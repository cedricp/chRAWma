#include "scale_processor.h"

#include <GL/glew.h>
#include "glsl_shader.h"

struct scale_impl{
	render::Shader _program;
};

const char* compute_scale_shader = "#version 440\n"
"layout (binding=0, rgba16f) uniform writeonly image2D output_buffer;\n"
"layout (binding=1) uniform sampler2D input_buffer;\n"
"layout (local_size_x = 16, local_size_y = 16) in;\n"
"void main(){\n"
"   ivec2 sz = imageSize(output_buffer);\n"
"   ivec2 loadPos = ivec2(gl_GlobalInvocationID.xy);\n"
"   vec2 uvPos = vec2(float(loadPos.x) / float(sz.x), float(loadPos.y) / float(sz.y));"
"   vec4 col = texture(input_buffer, uvPos);\n"
"   imageStore(output_buffer, loadPos, col);\n"
"}\n";


Scale_processor::Scale_processor()
{
    _imp = new scale_impl;
    std::string shader = compute_scale_shader;
    _imp->_program.init_from_string(shader);
}

Scale_processor::~Scale_processor()
{
    delete _imp;
}

void Scale_processor::process(const Texture2D& in_tex, Texture2D& out_tex)
{
	_imp->_program.bind();
	glBindImageTexture(0, out_tex.get_gltex(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, in_tex.get_gltex());
	glDispatchCompute(out_tex.width() / 16 + 1, out_tex.height() / 16 + 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	_imp->_program.unbind();
}