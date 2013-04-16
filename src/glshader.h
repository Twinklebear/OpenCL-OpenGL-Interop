#ifndef GLSHADER_H
#define GLSHADER_H

#include <string>
#include <SDL_opengl.h>
#include "handle.h"

namespace GL {
    /**
    * Handles and simplifies interacting with OpenGL Shader objects
    */
    class Shader {
    public:
        //We want type checking of the value passed to make sure it's
        //a valid shader type flag
        enum TYPE { FRAGMENT = GL_FRAGMENT_SHADER, VERTEX = GL_VERTEX_SHADER, 
            GEOMETRY = GL_GEOMETRY_SHADER };
    public:
        /**
        * Create a new shader of the desired type using the contents
        * of file as the shader source code
        * @param file The shader source filename
        * @param type The type of shader to create
        */
        Shader(std::string file, TYPE type);
        /**
        * Debug a shader, check if it compiled successfully and get back
        * the shader info log
        * @return The shader info log
        */
        std::string Debug();
        /**
        * Check the status of compiling the shader, True if ok
        * @return True if compiled successfully
        */
        bool Status();
        /**
        * Implicitly convert to a GLuint if trying to use
        * the shader as such
        */
        operator GLuint();

    private:
        /**
        * Create a shader program of some type using the contents
        * of some file
        * @param file The file to read contents from
        * @param type The type of shader to create
        */
        void Create(const std::string &file);
    
    private:
        Handle mHandle;
        TYPE mType;
		//The shader deleter function
		const static std::function<void(GLuint*)> sShaderDelete;
    };
}

#endif