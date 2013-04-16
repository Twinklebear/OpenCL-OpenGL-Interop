#include <SDL.h>
#include <SDL_opengl.h>
#include "glfunctions.h"

void GL::SetupGLFunctions(){
    //For interacting with VBOs
    SetProcAddress(GenBuffers, "glGenBuffers");
    SetProcAddress(DeleteBuffers, "glDeleteBuffers");
    SetProcAddress(BindBuffer, "glBindBuffer");
    SetProcAddress(BufferData, "glBufferData");
    //For interacting with VAOs
    SetProcAddress(GenVertexArrays, "glGenVertexArrays");
    SetProcAddress(DeleteVertexArrays, "glDeleteVertexArrays");
    SetProcAddress(BindVertexArray, "glBindVertexArray");
    //For interacting with Shaders
    SetProcAddress(CreateShader, "glCreateShader");
    SetProcAddress(DeleteShader, "glDeleteShader");
    SetProcAddress(ShaderSource, "glShaderSource");
    SetProcAddress(CompileShader, "glCompileShader");
    SetProcAddress(GetShaderiv, "glGetShaderiv");
    SetProcAddress(GetShaderInfoLog, "glGetShaderInfoLog");
    //For interacting with Shader Programs
    SetProcAddress(CreateProgram, "glCreateProgram");
    SetProcAddress(DeleteProgram, "glDeleteProgram");
    SetProcAddress(AttachShader, "glAttachShader");
    SetProcAddress(DetachShader, "glDetachShader");
    SetProcAddress(LinkProgram, "glLinkProgram");
    SetProcAddress(GetProgramiv, "glGetProgramiv");
    SetProcAddress(GetProgramInfoLog, "glGetProgramInfoLog");
    SetProcAddress(UseProgram, "glUseProgram");
    //For interacting with program attributes and uniforms
    SetProcAddress(GetAttribLocation, "glGetAttribLocation");
    SetProcAddress(VertexAttribPointer, "glVertexAttribPointer");
    SetProcAddress(EnableVertexAttribArray, "glEnableVertexAttribArray");
    SetProcAddress(DisableVertexAttribArray, "glDisableVertexAttribArray");
    SetProcAddress(GetUniformLocation, "glGetUniformLocation");
	SetProcAddress(Uniform1f, "glUniform1f");
	SetProcAddress(Uniform3fv, "glUniform3fv");
	SetProcAddress(Uniform4fv, "glUniform4fv");
    SetProcAddress(UniformMatrix4fv, "glUniformMatrix4fv");
	//Texture functions
	SetProcAddress(ActiveTexture, "glActiveTexture");
	SetProcAddress(GenerateMipmap, "glGenerateMipmap");
}
//For interacting with VBOs
PFNGLGENBUFFERSPROC GL::GenBuffers = nullptr;
PFNGLDELETEBUFFERSPROC GL::DeleteBuffers = nullptr;
PFNGLBINDBUFFERPROC GL::BindBuffer = nullptr;
PFNGLBUFFERDATAPROC GL::BufferData = nullptr;
//For interacting with VAOs
PFNGLGENVERTEXARRAYSPROC GL::GenVertexArrays = nullptr;
PFNGLDELETEVERTEXARRAYSPROC GL::DeleteVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC GL::BindVertexArray = nullptr;
//For interacting with Shaders
PFNGLCREATESHADERPROC GL::CreateShader = nullptr;
PFNGLDELETESHADERPROC GL::DeleteShader = nullptr;
PFNGLSHADERSOURCEPROC GL::ShaderSource = nullptr;
PFNGLCOMPILESHADERPROC GL::CompileShader = nullptr;
PFNGLGETSHADERIVPROC GL::GetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC GL::GetShaderInfoLog = nullptr;
//For interacting with Shader Programs
PFNGLCREATEPROGRAMPROC GL::CreateProgram = nullptr;
PFNGLDELETEPROGRAMPROC GL::DeleteProgram = nullptr;
PFNGLATTACHSHADERPROC GL::AttachShader = nullptr;
PFNGLDETACHSHADERPROC GL::DetachShader = nullptr;
PFNGLLINKPROGRAMPROC GL::LinkProgram = nullptr;
PFNGLGETPROGRAMIVPROC GL::GetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC GL::GetProgramInfoLog = nullptr;
PFNGLUSEPROGRAMPROC GL::UseProgram = nullptr;
//For interacting with program attributes and uniforms
PFNGLGETATTRIBLOCATIONPROC GL::GetAttribLocation = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC GL::VertexAttribPointer = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC GL::EnableVertexAttribArray = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC GL::DisableVertexAttribArray = nullptr;
PFNGLGETUNIFORMLOCATIONPROC GL::GetUniformLocation = nullptr;
PFNGLUNIFORM1FPROC GL::Uniform1f = nullptr;
PFNGLUNIFORM3FVPROC GL::Uniform3fv = nullptr;
PFNGLUNIFORM4FVPROC GL::Uniform4fv = nullptr;
PFNGLUNIFORMMATRIX4FVPROC GL::UniformMatrix4fv = nullptr;
//Texture functions
PFNGLACTIVETEXTUREPROC GL::ActiveTexture = nullptr;
PFNGLGENERATEMIPMAPPROC GL::GenerateMipmap = nullptr;
