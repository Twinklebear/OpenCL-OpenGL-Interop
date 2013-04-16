#ifndef GL_HANDLE_H
#define GL_HANDLE_H

#include <functional>
#include <memory>
#include <SDL_opengl.h>

namespace GL {
	/**
	* It's really just a shared_ptr that will implicitly convert to a GLuint
	*/
    class Handle {
    public:
        //Mark mObj as a nullptr
        Handle();
        //Construct the handler, giving it an object to handle and a destruction function
        Handle(GLuint obj, std::function<void(GLuint*)> del);
        //Behave like the stored object (GLuint) implicitly
        operator GLuint();
        //Release the reference
        void Release();

    private:
		std::shared_ptr<GLuint> mObj;
    };
}

#endif