#include "scale_processor.h"

#include <GL/glew.h>
#include "glsl_shader.h"

struct scale_impl{
	render::Shader _shader;
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
    _imp->_shader.init_from_string(shader);
}

Scale_processor::~Scale_processor()
{
    delete _imp;
}

void Scale_processor::process(const Texture2D& in_tex, Texture2D& out_tex)
{
	int tw = (out_tex.width() + 15) / 16;
	int th = (out_tex.height() + 15) / 16;

	_imp->_shader.bind();
	out_tex.bindImageTexture(0);
	in_tex.bindTexture(1);

	_imp->_shader.dispatchCompute(tw, th);
	_imp->_shader.enableMemoryBarrier();
	
	_imp->_shader.unbind();
}