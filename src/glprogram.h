#ifndef GLPROGRAM_H
#define GLPROGRAM_H

#include <string>
#include <SDL_opengl.h>
#include <glm/glm.hpp>
#include "handle.h"
#include "glshader.h"
#include "glvertexarray.h"

namespace GL {
    /**
    * Handles and simplifies interaction with OpenGL
    * shader program objects
    */
    class Program {
    public:
		///Default constructor, will be an invalid handle
		Program();
        /**
        * Create a shader program using vertex and fragment shader files
        * @param vertex The vertex shader file
        * @param frag The fragment shader file
        */
        Program(std::string vertex, std::string frag);
        /**
        * Create a shader program from existing vertex and fragment shaders
        * The function assumes that the shaders being passed have 
        * been compiled successfully
        * @param vertex The vertex shader
        * @param frag The fragment shader
        */
        Program(Shader &vert, Shader &frag);
		/**
		* Link the program from two shaders
		* @param vert Vertex shader
		* @param frag Fragment shader
		*/
		void Link(Shader &vert, Shader &frag);
        /**
        * Debug a program, check if it linked successfully and return the 
        * program log
        * @return The shader info log
        */
        std::string Debug();
        /**
        * Check the status of compiling the shader, True if ok
        * @return True if compiled successfully
        */
        bool Status();
        /**
        * Get the location of an attribute with some name
        * @param name The name of the attribute to get the location of
        */
        GLint GetAttribute(std::string name);
        //TODO: Come up with a better idea for making all these methods
        //or can i?
		///Pass a float as a uniform to some attrib
		void Uniform1f(const std::string &attrib, float f);
		/**
		* Pass a glm::vec3 as a uniform
		* @param attrib Name of the attribute to set
		* @param vec Vector to be passed
		*/
		void Uniform3fv(const std::string &attrib, const glm::vec3 &vec);
        /**
        * Pass a glm::vec4 as a uniform 
        * @param attrib the name of the attrib to set
        * @param vec The vector to be passed
        */
        void Uniform4fv(const std::string &attrib, const glm::vec4 &vec);
        /**
        * Pass a glm::mat4x4 as a uniform attribute
        * @param attrib The name of the uniform to set
        * @param matrix The matrix to be passed
        */
        void UniformMat4x4(const std::string &attrib, const glm::mat4 &matrix);
        /**
        * Implicitly convert to a GLuint if trying to use
        * the program as such
        */
        operator GLuint();

    private:
        Handle mHandle;
		//Deleter function for the handle
		const static std::function<void(GLuint*)> sProgDelete;
    };
}

#endif