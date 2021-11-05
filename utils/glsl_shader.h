// ========
// shader.h
// ========

#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>

namespace render
{

	void init();
	class Shader
	{
		unsigned int _program;
		unsigned int _shaders[2];

		std::string loadShader(const std::string& fileName);
		unsigned int createShader(const std::string& text, unsigned int type);
		void linkProgram();

	public:
		Shader();
		Shader(std::string vs, std::string fs, bool isfile = false);
		Shader(std::string cs, bool isfile = false);
		~Shader();

		Shader(const Shader& s){
			_program = s._program;
			_shaders[0] = s._shaders[0];
			_shaders[1] = s._shaders[1];
		}

		void init(std::string& file_name_vs, std::string& file_name_fs);
		void init_from_string(std::string& cs);
		void bind();
		void unbind();

		int getUniformLocation(std::string param);

		void DispatchCompute(int x, int y, int z);
		void EnableMemoryBarrier();
	};
}

#endif
