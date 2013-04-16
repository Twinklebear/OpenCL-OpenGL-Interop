#ifndef GLFUNCTIONS_H
#define GLFUNCTIONS_H

#include <string>
#include <SDL_opengl.h>

/**
* Namespace to contain various classes/functions for interacting
* with OpenGL
*/
namespace GL {
    ///Get the functions from SDL_GL_GetProcAddress
    void SetupGLFunctions();
    //Take a PFNGL function pointer and set it up
    template<class T>
    void SetProcAddress(T &func, std::string proc){
        func = (T)SDL_GL_GetProcAddress(proc.c_str());
    }
    //The GL Functions
    //For interacting with VBOs
    extern PFNGLGENBUFFERSPROC GenBuffers;
    extern PFNGLDELETEBUFFERSPROC DeleteBuffers;
    extern PFNGLBINDBUFFERPROC BindBuffer;
    extern PFNGLBUFFERDATAPROC BufferData;
    //For interacting with VAOs
    extern PFNGLGENVERTEXARRAYSPROC GenVertexArrays;
    extern PFNGLDELETEVERTEXARRAYSPROC DeleteVertexArrays;
    extern PFNGLBINDVERTEXARRAYPROC BindVertexArray;
    //For interacting with Shaders
    extern PFNGLCREATESHADERPROC CreateShader;
    extern PFNGLDELETESHADERPROC DeleteShader;
    extern PFNGLSHADERSOURCEPROC ShaderSource;
    extern PFNGLCOMPILESHADERPROC CompileShader;
    extern PFNGLGETSHADERIVPROC GetShaderiv;
    extern PFNGLGETSHADERINFOLOGPROC GetShaderInfoLog;
    //For interacting with Shader Programs
    extern PFNGLCREATEPROGRAMPROC CreateProgram;
    extern PFNGLDELETEPROGRAMPROC DeleteProgram;
    extern PFNGLATTACHSHADERPROC AttachShader;
    extern PFNGLDETACHSHADERPROC DetachShader;
    extern PFNGLLINKPROGRAMPROC LinkProgram;
    extern PFNGLGETPROGRAMIVPROC GetProgramiv;
    extern PFNGLGETPROGRAMINFOLOGPROC GetProgramInfoLog;
    extern PFNGLUSEPROGRAMPROC UseProgram;
    //For interacting with program attributes and uniforms
    extern PFNGLGETATTRIBLOCATIONPROC GetAttribLocation;
    extern PFNGLVERTEXATTRIBPOINTERPROC VertexAttribPointer;
    extern PFNGLENABLEVERTEXATTRIBARRAYPROC EnableVertexAttribArray;
    extern PFNGLDISABLEVERTEXATTRIBARRAYPROC DisableVertexAttribArray;
    extern PFNGLGETUNIFORMLOCATIONPROC GetUniformLocation;
	extern PFNGLUNIFORM1FPROC Uniform1f;
	extern PFNGLUNIFORM4FVPROC Uniform4fv;
	extern PFNGLUNIFORM3FVPROC Uniform3fv;
    extern PFNGLUNIFORMMATRIX4FVPROC UniformMatrix4fv;
	//Texture function
	extern PFNGLACTIVETEXTUREPROC ActiveTexture;
	extern PFNGLGENERATEMIPMAPPROC GenerateMipmap;
}

#endif