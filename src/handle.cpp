#include <functional>
#include <SDL_opengl.h>
#include "handle.h"

GL::Handle::Handle() : mObj(nullptr) {
	mObj = std::shared_ptr<GLuint>(new GLuint(-1));
}
GL::Handle::Handle(GLuint obj, std::function<void(GLuint*)> del) 
	: mObj(nullptr)
{
	mObj = std::shared_ptr<GLuint>(new GLuint(obj), del);
}
void GL::Handle::Release(){
	mObj.reset();
}
GL::Handle::operator GLuint(){
    return *mObj.get();
}
