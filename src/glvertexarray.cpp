#include <vector>
#include <functional>
#include <SDL_opengl.h>
#include "handle.h"
#include "glfunctions.h"
#include "glvertexbuffer.h"
#include "glvertexarray.h"

#include <iostream>

const std::function<void(GLuint*)> GL::VertexArray::sVaoDeleter = 
	[](GLuint *vao){ GL::DeleteVertexArrays(1, vao); };

GL::VertexArray::VertexArray(){
	//Generate VAO, associate with the buffer and assign it to the handle
	GLuint vao;
	GL::GenVertexArrays(1, &vao);
	mHandle = Handle(vao, sVaoDeleter);
}
void GL::VertexArray::Reference(VertexBuffer &b, const std::string &name){
	GL::BindVertexArray(mHandle);
	GL::BindBuffer(b.Type(), b);
	GL::BindBuffer(b.Type(), 0);
	//Store a reference to the vbo to keep it alive
	mVbos[name] = b;
}
void GL::VertexArray::ElementBuffer(std::vector<unsigned short> indices){
	//TODO: Am I not associating the element buffer correctly? The constructor
	//will bind the buffer so shouldn't the vao pick it up?
	GL::BindVertexArray(mHandle);
	mVbos["elem"] = GL::VertexBuffer(indices, GL::BUFFER::ELEMENT);
	GLuint err = glGetError();
	if (err != GL_NO_ERROR)
		std::cout << "gl error #: " << err << std::endl;
}
size_t GL::VertexArray::NumElements(const std::string &name){
	std::map<std::string, VertexBuffer>::iterator vbo = mVbos.find(name);
	if (vbo == mVbos.end())
		return -1;
	return vbo->second.NumVals();
}
void GL::VertexArray::SetAttribPointer(const std::string &name, GLint attrib, size_t size, GLenum type,
			bool normalized, size_t stride, void *offset)
{
	//Make sure it's a vbo we've stored
	std::map<std::string, VertexBuffer>::iterator vbo = mVbos.find(name);
	if (vbo == mVbos.end())
		throw std::runtime_error("Invalid vbo name: " + name);

	GL::BindVertexArray(mHandle);
	GL::BindBuffer(vbo->second.Type(), vbo->second);
	GL::EnableVertexAttribArray(attrib);
	GL::VertexAttribPointer(attrib, size, type, normalized, stride, offset);
}
GL::VertexArray::operator GLuint(){
	return mHandle;
}
