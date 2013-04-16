#ifndef GLVERTEXARRAY_H
#define GLVERTEXARRAY_H

#include <map>
#include <functional>
#include <SDL_opengl.h>
#include "handle.h"
#include "glvertexbuffer.h"

namespace GL {
	/**
	* Handles and simplifies interacting with OpenGL VBOs
	*/
	class VertexArray {
	public:
		/**
		* Create a VAO for use
		*/
		VertexArray();
		/**
		* Reference a vbo object and associate it with some name
		* @param b VBO to reference
		* @param name Name to associate with the vbo
		*/
		void Reference(VertexBuffer &b, const std::string &name);
		/**
		* Set the element buffer for indexed rendering, if desired
		* @param indices The indices to use
		*/
		void ElementBuffer(std::vector<unsigned short> indices);
		/**
		* Get the number of elements in a buffer
		* @param name Name of the buffer to get # of elements of
		* @return # of elements, or -1 if no buffer with that name
		*/
		size_t NumElements(const std::string &name);
		/**
		* Set some attribute to use values taken from a vbo given some
		* size of components to take, a type, if the values are normalize
		* a byte stride and an offset
		* @param name The name of the buffer stored to use as the data
		* @param attrib The name of the attribute to set
		* @param vao The vertex array to get the values from
		* @param size The number of components per attribute
		* @param type The type of data in the vbo
		* @param normalized Specifies that fixed point values should be normalized if true, default false
		* @param stried The byte offset between attributes, default 0
		* @param offset The byte offset to begin reading attributes, default 0
		*/
		void SetAttribPointer(const std::string &name, GLint attrib, size_t size, GLenum type,
			bool normalized = GL_FALSE, size_t stride = 0, void *offset = 0);
		/**
		* Implicitly convert to a GLuint if trying to use
		* the program as such
		*/
		operator GLuint();

	private:
		///The VAO handle
		Handle mHandle;
		///The VBOs the VAO is referencing
		std::map<std::string, VertexBuffer> mVbos;
		///A function to delete a single vao
		const static std::function<void(GLuint*)> sVaoDeleter;
	};
}

#endif