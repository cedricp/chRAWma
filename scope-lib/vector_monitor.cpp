#include "vector_monitor.h"
#include <GL/glew.h>

std::string vector_cmp = "#version 440\n\
layout (binding = 0, r32ui) uniform coherent uimage2D destTex;\n\
layout (binding = 1, rgba16f) uniform readonly image2D sourceTex;\n\
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;\n\
\n\
vec4 RGB_BT709_2_YUV(vec4 rgb)\n\
{\n\
	vec4 yuv;\n\
    yuv.x = 0.2126f * rgb.x + 0.7152f * rgb.y + 0.0722f * rgb.z;\n\
    yuv.y = -0.09991f * rgb.x - 0.33609f * rgb.y + 0.436f * rgb.z;;\n\
    yuv.z = 0.615f * rgb.x - 0.55851f * rgb.y - 0.05639f * rgb.z;;\n\
//    yuv.y = (rgb.z - yuv.x) * (1./ 1.8556);\n\
//    yuv.z = (rgb.x - yuv.x) * (1. / 1.5748);\n\
    return yuv;\n\
}\n\
\n\
vec4 RGB_BT601_2_YUV(vec4 rgb)\n\
{\n\
	vec4 yuv;\n\
    yuv.x = 0.299 * rgb.x + 0.587 * rgb.y + 0.114 * rgb.z;\n\
    yuv.y = (rgb.z - yuv.x) * (1./ 1.772);\n\
    yuv.z = (rgb.x - yuv.x) * (1. / 1.402);\n\
    return yuv;\n\
}\n\
\n\
vec4 RGB_BT2020_2_YUV(vec4 rgb)\n\
{\n\
	vec4 yuv;\n\
    yuv.x = 0.2627f * rgb.x + 0.6780 * rgb.y + 0.0593 * rgb.z;\n\
    yuv.y = (rgb.z - yuv.x) * (1. / 1.8814);\n\
    yuv.z = (rgb.x - yuv.x) * (1. / 1.4747);\n\
    return yuv;\n\
}\n\
\n\
vec3 rgb2hsv(vec3 c)\n\
{\n\
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);\n\
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));\n\
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));\n\
\n\
    float d = q.x - min(q.w, q.y);\n\
    float e = 1.0e-10;\n\
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);\n\
}\n\
\n\
vec3 hsv2rgb(vec3 c)\n\
{\n\
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);\n\
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n\
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n\
}\n\
\n\
void main() \n\
{\n\
	ivec2 imgsize = imageSize(sourceTex);\n\
	ivec2 scopesize = imageSize(destTex);\n\
	ivec2 loadPos = ivec2(gl_GlobalInvocationID.xy);\n\
	\n\
	vec4 col = imageLoad(sourceTex, loadPos);\n\
	vec4 yuv = RGB_BT709_2_YUV(col) * 2.0;\n\
	\n\
	//yuv.y *= 255.0f / (122.0f * 2.0f);\n\
    //yuv.z *= 255.0f / (157.0f * 2.0f);\n\
    \n\
    int dest_x = int((yuv.y + 0.5f) * float(scopesize.x));\n\
    int dest_y = int((-yuv.z + 0.5f) * float(scopesize.y));\n\
	\n\
	ivec2 storePos = ivec2(dest_x, dest_y);\n\
	\n\
	memoryBarrier();\n\
	\n\
	imageAtomicAdd(destTex, storePos, 0x1);\n\
\n\
}";


std::string vector_mix = "#version 440\n\
layout (binding = 0, r32ui) uniform readonly uimage2D source;\n\
layout (binding = 1, rgba16f) uniform writeonly image2D dest;\n\
layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;\n\
\n\
uniform float spot_intensity;\n\
uniform float scale_y;\n\
uniform int color_wheel;\n\
\n\
vec4 YUV2RGB(vec4 yuv)\n\
{\n\
	vec4 rgb;\n\
    rgb.x = yuv.x + 1.28033f * yuv.z;\n\
    rgb.y = yuv.x + -0.21482f * yuv.y - 0.8059f * yuv.z;\n\
    rgb.z = yuv.x + 2.12798f * yuv.y;;\n\
    return rgb;\n\
}\n\
\n\
void main()\n\
{\n\
	ivec2 size = imageSize(source);\n\
	ivec2 center = size / 2;\n\
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);\n\
	uvec4 col = imageLoad(source, storePos);\n\
	\n\
	vec4 finalColor =vec4(0.,0.,0.,1.);\n\
	\n\
	if (color_wheel > 0){\n\
		vec2 v = vec2(center) - vec2(storePos);\n\
		float lenv = length(v);\n\
		if (lenv < float(center.y))\n\
		{\n\
			if ((storePos.x < center.x) && (storePos.y < center.y)){\n\
				float angle = acos( dot( vec2(0.,1.), normalize(vec2(v)) ) );\n\
				if (angle > 0.558505 && angle < 0.593412){\n\
					finalColor.x += 0.01;\n\
					finalColor.y += 0.01;\n\
					finalColor.z += 0.01;\n\
				}\n\
			}\n\
			\n\
			float u = v.x / float(size.x);\n\
			float v = v.y / float(size.y);\n\
			float y = lenv / float(center.y) * 0.75;\n\
			vec4 rgb = YUV2RGB(vec4(1.-y, -u, v, 0.0));\n\
			finalColor.x += rgb.x * 0.1;\n\
			finalColor.y += rgb.y * 0.1;\n\
			finalColor.z += rgb.z * 0.1;\n\
		}\n\
	} else {\n\
		if (size.y - storePos.y == int(float(size.y) * 0.5f * scale_y)){\n\
			finalColor.x += 0.1f;\n\
			finalColor.y += 0.1f;\n\
			finalColor.z += 0.1f;\n\
		}\n\
	}\n\
	\n\
	finalColor.xyz += float(col.x) / (1024.0 / spot_intensity);\n\
	\n\
	finalColor.x = pow(finalColor.x, 0.4545f);\n\
	finalColor.y = pow(finalColor.y, 0.4545f);\n\
	finalColor.z = pow(finalColor.z, 0.4545f);\n\
	\n\
	imageStore(dest, storePos, finalColor);\n\
}";

vectorMonitor::vectorMonitor(int w, int h) : _height(h), _width(w),
												 _compute_shader(vector_cmp),
												 _compute_shader_mix(vector_mix)
{
	_inh = _inw = 0;
	prepareOutput_textures();

	_mix_scale_loc = _compute_shader_mix.getUniformLocation("scale_y");
	_mix_spot_loc = _compute_shader_mix.getUniformLocation("spot_intensity");
    _mix_colorwheel_loc = _compute_shader_mix.getUniformLocation("color_wheel");
}

vectorMonitor::~vectorMonitor()
{
}

const Texture2D& vectorMonitor::compute(const Texture2D& tex)
{
	_inw = tex.width();
	_inh = tex.height();

	if (!_inw || !_inh){
		return _output_texture;
	}

	clearTextures();

	_compute_shader.bind();
    _intermediate_texture.bindImageTexture(0, GL_READ_WRITE);
    tex.bindImageTexture(1, GL_READ_ONLY);
	_compute_shader.dispatchCompute(_inw / 8, _inh / 8);
	_compute_shader.enableMemoryBarrier();
	_compute_shader.unbind();

	_compute_shader_mix.bind();
    _intermediate_texture.bindImageTexture(0, GL_READ_ONLY);
    _output_texture.bindImageTexture(1, GL_WRITE_ONLY);
	glUniform1f(_mix_spot_loc, _spot_intensity);
	glUniform1f(_mix_scale_loc, _scale);
    glUniform1i(_mix_colorwheel_loc, 1);
	_compute_shader.dispatchCompute(_width / 16, _height / 16);
	_compute_shader.enableMemoryBarrier();
	_compute_shader_mix.unbind();

	return _output_texture;
}

void vectorMonitor::prepareOutput_textures()
{
	_intermediate_texture.init(GL_RED_INTEGER, GL_UNSIGNED_INT, _width, _height);
	_output_texture.init(GL_RGBA, GL_HALF_FLOAT, _width, _height);
}

void vectorMonitor::clearTextures()
{
	_intermediate_texture.clear(GL_RED_INTEGER, GL_UNSIGNED_INT);
	_output_texture.clear(GL_RGBA, GL_HALF_FLOAT);
}
