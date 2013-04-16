#ifndef GLVERTEXBUFFER_H
#define GLVERTEXBUFFER_H

#include <functional>
#include <array>
#include <vector>
#include <SDL_opengl.h>
#include <glm/glm.hpp>
#include "handle.h"
#include "glfunctions.h"

#include <iostream>

namespace GL {
	enum BUFFER { ARRAY = GL_ARRAY_BUFFER, ELEMENT = GL_ELEMENT_ARRAY_BUFFER };
	/**
	* Handles and simplifies interacting with OpenGL VBOs
	*/
	class VertexBuffer {
	public:
		///Blank constructor, literally does nothing, but it seems I need to define it
		VertexBuffer() : mNvals(0) {};
		///Create an empty buffer of some size
		VertexBuffer(size_t size, BUFFER type = BUFFER::ARRAY) : mType(type) {
			GLuint vbo;
			GenBuffers(1, &vbo);
			mHandle = Handle(vbo, sVboDeleter);
			BindBuffer(mType, mHandle);
			//Will want to enable DYNAMIC draw and other modes so I can change
			//the buffers/images repeatedly will using them
			BufferData(mType, size, NULL, GL_STATIC_DRAW);
		}
		/**
		* Create a new VBO using the data stored in the array passed
		* Currently just doing with GL_STATIC_DRAW since that's all I need but 
		* I'll probably expand to handle all types later
		* @param data The array of data to be passed
		* @param type Type of buffer to create, defaults to array buffer
		*/
		template<size_t N>
		VertexBuffer(const std::array<float, N> &data, BUFFER type = BUFFER::ARRAY)
			: mType(type), mNvals(N)
		{
			GLuint vbo;
			GenBuffers(1, &vbo);
			mHandle = Handle(vbo, sVboDeleter);
			BindBuffer(mType, mHandle);
			BufferData(mType, N * sizeof(float), &data[0], GL_STATIC_DRAW);
		}
		/**
		* Create a new VBO using the data stored in the vector passed
		* @param data Vertex data to buffer
		* @param type Type of buffer to create, defaults to array buffer
		*/
		VertexBuffer(const std::vector<glm::vec3> &data, BUFFER type = BUFFER::ARRAY)
			: mType(type), mNvals(data.size())
		{
			GLuint vbo;
			GenBuffers(1, &vbo);
			mHandle = Handle(vbo, sVboDeleter);
			BindBuffer(mType, mHandle);
			BufferData(mType, data.size() * sizeof(glm::vec3), &data[0], GL_STATIC_DRAW);
		}
		/**
		* Create a new VBO using the data in the vector
		* @param data Vertex data to buffer
		* @param type Type of buffer to create, defaults to array buffer
		*/
		template<class T>
		VertexBuffer(const std::vector<T> &data, BUFFER type = BUFFER::ARRAY)
			: mType(type), mNvals(data.size())
		{
			GLuint vbo;
			GenBuffers(1, &vbo);
			mHandle = Handle(vbo, sVboDeleter);
			BindBuffer(mType, mHandle);
			BufferData(mType, data.size() * sizeof(T), &data[0], GL_STATIC_DRAW);
		}
		/**
		* Tell the VBO to handle interaction with an existing object
		* @param vbo The existing vbo to take ownership of
		* @param type The type of buffer being managed
		*/
		void Manage(GLuint vbo, BUFFER type = BUFFER::ARRAY){
			mHandle = Handle(vbo, sVboDeleter);
			mType = type;
		}
		/**
		* Get the buffer type 
		* @return the buffer type
		*/
		BUFFER Type(){
			return mType;
		}
		/**
		* Get the number of values
		*/
		size_t NumVals(){
			return mNvals;
		}
		/**
		* Implicitly convert to a GLuint if trying to use
		* the program as such
		*/
		operator GLuint(){
			return mHandle;
		}

	private:
		Handle mHandle;
		BUFFER mType;
		///Number of values in the buffer
		size_t mNvals;
		/**
		* Wrapper around the delete buffers function to make a delete 1 vbo
		* function that can be passed as a handle destructor
		*/
		static const std::function<void(GLuint*)> sVboDeleter;
	};    
}

#endif