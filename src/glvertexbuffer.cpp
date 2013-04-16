#include <functional>
#include <array>
#include <SDL_opengl.h>
#include "handle.h"
#include "glvertexbuffer.h"

const std::function<void(GLuint*)> GL::VertexBuffer::sVboDeleter = [](GLuint *b){
	GL::DeleteBuffers(1, b);
};
