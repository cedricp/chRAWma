#include "histogram_monitor.h"
#include <GL/glew.h>

static std::string compute_src = "#version 440\n\
\n\
layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;\n\
layout (binding = 0, rgba16f) uniform readonly image2D sourceTex;\n\
layout(std430, binding = 1) coherent buffer dataLayout\n\
{\n\
	int[] histdata;\n\
};\n\
\n\
uniform int scope_w;\n\
\n\
void main() \n\
{\n\
	ivec2 loadPos = ivec2(gl_GlobalInvocationID.xy);\n\
	\n\
	vec4 col = imageLoad(sourceTex, loadPos);\n\
	float luma  = 0.299 * col.x + 0.587 * col.y + 0.114 * col.z;\n\
	\n\
	col.x = col.x > 1.0 ? 1.0 : col.x;\n\
	col.y = col.y > 1.0 ? 1.0 : col.y;\n\
	col.z = col.z > 1.0 ? 1.0 : col.z; \n\
	luma = luma > 1.0 ? 1.0 : luma; \n\
\n\
	int storePosR = int(col.x * float(scope_w));\n\
	int storePosG = int(col.y * float(scope_w));\n\
	int storePosB = int(col.z * float(scope_w));\n\
	int storePosY = int(luma * float(scope_w));\n\
	\n\
	storePosR = storePosR > scope_w ? scope_w : storePosR;\n\
	storePosG = storePosG > scope_w ? scope_w : storePosG;\n\
	storePosB = storePosB > scope_w ? scope_w : storePosB;\n\
	storePosY = storePosY > scope_w ? scope_w : storePosY;\n\
	\n\
	atomicAdd(histdata[storePosR * 4], 1);\n\
	atomicAdd(histdata[storePosG * 4 + 1], 1);\n\
	atomicAdd(histdata[storePosB * 4 + 2], 1);\n\
	atomicAdd(histdata[storePosY * 4 + 3], 1);\n\
}\n";

static std::string mix_src = "#version 440\n\
\n\
layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;\n\
layout (binding = 0, rgba16f) uniform writeonly image2D destTex;\n\
layout(std430, binding = 1) buffer dataLayout\n\
{\n\
	int[] histdata;\n\
};\n\
\n\
uniform float scale;\n\
\n\
void main() \n\
{\n\
	ivec2 scopesize = imageSize(destTex);\n\
	ivec2 loadPos = ivec2(gl_GlobalInvocationID.xy);\n\
	\n\
	int YPos = scopesize.y - loadPos.y;\n\
	\n\
	float R = float(histdata[loadPos.x * 4]) * scale;\n\
	float G = float(histdata[loadPos.x * 4 + 1]) * scale;\n\
	float B = float(histdata[loadPos.x * 4 + 2]) * scale;\n\
	float Y = float(histdata[loadPos.x * 4 + 3]) * scale;\n\
	\n\
	// Log scale\n\
	R = log2(R + 1) * scopesize.y;\n\
	G = log2(G + 1) * scopesize.y;\n\
	B = log2(B + 1) * scopesize.y;\n\
	Y = log2(Y + 1) * scopesize.y;\n\
	\n\
	vec4 finalCol = vec4(0.,0.,0.,0.);\n\
	\n\
	if (YPos < R){\n\
		finalCol += vec4(0.2, 0., 0., 0.);\n\
        finalCol.w = 1.0;\n\
	}\n\
	\n\
	if (YPos < G){\n\
		finalCol += vec4(0., 0.2, 0., 0.);\n\
        finalCol.w = 1.0;\n\
	}\n\
	\n\
	if (YPos < B){\n\
		finalCol += vec4(0., 0., 0.2, 0.);\n\
        finalCol.w = 1.0;\n\
	}\n\
	\n\
	if (YPos < Y){\n\
		finalCol += vec4(0.3, 0.3, 0.3, 1.);	\n\
        if (YPos > Y - 2){\n\
            finalCol += vec4(0.2, 0.2, 0.2, 1.);	\n\
        }\n\
        finalCol.w = 1.0;\n\
	}\n\
	\n\
	imageStore(destTex, loadPos, finalCol);	\n\
}";

histogramMonitor::histogramMonitor(int w, int h) : _height(h), _width(w),
												 _compute_shader(compute_src),
												 _compute_shader_mix(mix_src),
                                                 _cpu_buffer(NULL)
{
	_inh = _inw = 0;
    prepareOutput_textures();

    _histogram_width_loc = _compute_shader.getUniformLocation("scope_w");
    _histogram_scale_loc = _compute_shader_mix.getUniformLocation("scale");
}

histogramMonitor::~histogramMonitor()
{
    if (_cpu_buffer){
        free(_cpu_buffer);
    }
    if (_histogram_buffer_id){
		glDeleteBuffers(1, &_histogram_buffer_id);
	}
}

const Texture2D& histogramMonitor::compute(const Texture2D& tex)
{
	_inw = tex.width();
	_inh = tex.height();

	if (!_inw || !_inh){
		return _output_texture;
	}

	clearTextures();

    _compute_shader.bind();
    glUniform1i(_histogram_width_loc, _width);
    tex.bindImageTexture(0, GL_READ_ONLY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _histogram_buffer_id);
    _compute_shader.dispatchCompute(_inw / 16, _inh / 16);
    _compute_shader.enableMemoryBarrier();
    glGetNamedBufferSubData(_histogram_buffer_id, 0, sizeof(uint32_t)*4*_width, _cpu_buffer);
    _compute_shader.unbind();

    uint32_t max = 0;
    for (int i = 0; i < 4*_width; ++i){
        if (_cpu_buffer[i] > max) max = _cpu_buffer[i];
    }

    _compute_shader_mix.bind();
    glUniform1f(_histogram_scale_loc, 1.0f / float(max));
    _output_texture.bindImageTexture(0, GL_READ_ONLY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _histogram_buffer_id);
    _compute_shader_mix.dispatchCompute(_width / 16, _height / 16);
    _compute_shader_mix.enableMemoryBarrier();
    _compute_shader_mix.unbind();

	return _output_texture;
}

void histogramMonitor::prepareOutput_textures()
{
    glGenBuffers(1, &_histogram_buffer_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _histogram_buffer_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * 4 * _width, NULL, GL_DYNAMIC_DRAW);
    _output_texture.init(GL_RGBA, GL_HALF_FLOAT, _width, _height);
    _cpu_buffer = (uint32_t*)malloc(sizeof(uint32_t) * 4 * _width);
}

void histogramMonitor::clearTextures()
{
    _output_texture.clear(GL_RGBA, GL_HALF_FLOAT);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, _histogram_buffer_id);
	glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_RGBA32UI, GL_RGBA, GL_UNSIGNED_INT, NULL);
}
