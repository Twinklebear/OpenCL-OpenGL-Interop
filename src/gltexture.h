#ifndef GLTEXTURE_H
#define GLTEXTURE_H

#include <string>
#include <functional>
#include <SDL_opengl.h>
#include "handle.h"

namespace GL {
	class Texture {
	public:
		///Blank constructor, will set handle to invalid (-1), needed for the 
		///material constructor
		Texture();
		/**
		* Load a texture from a file
		* @param file File to load from
		*/
		Texture(const std::string &file);
		/**
		* Load a texture into this object
		* @param file File to load texture from
		*/
		void Load(const std::string &file);
		///Pretend to be a GLuint
		operator GLuint();

	private:
		Handle mHandle;
		///The deleter function
		const static std::function<void(GLuint*)> sTexDeleter;
	};
}

#endif