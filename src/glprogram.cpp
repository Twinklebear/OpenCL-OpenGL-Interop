#include <string>
#include <vector>
#include <iostream>
#include <SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "handle.h"
#include "glfunctions.h"
#include "glshader.h"
#include "glprogram.h"

const std::function<void(GLuint*)> GL::Program::sProgDelete = 
	[](GLuint *i){ GL::DeleteProgram(*i); };

GL::Program::Program(){
}
GL::Program::Program(std::string vertex, std::string frag){
    Shader vertShader(vertex, Shader::TYPE::VERTEX);
    Shader fragShader(frag, Shader::TYPE::FRAGMENT);
    //Make sure the shaders are ok
    if (!vertShader.Status() || !fragShader.Status()){
        std::cout << vertShader.Debug() << std::endl
            << fragShader.Debug() << std::endl;
    }
	Link(vertShader, fragShader);
}
GL::Program::Program(Shader &vert, Shader &frag){
	Link(vert, frag);
}
void GL::Program::Link(Shader &vert, Shader &frag){
	mHandle = Handle(CreateProgram(), sProgDelete);
	AttachShader(mHandle, vert);
    AttachShader(mHandle, frag);
    LinkProgram(mHandle);
}
std::string GL::Program::Debug(){
    if (!Status()){
        //Get the log length and then get the log
        GLint logLength;
        GetShaderiv(mHandle, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength);
        GetShaderInfoLog(mHandle, logLength, NULL, &log[0]);
        
        return "Program errors:\n" + std::string(log.begin(), log.end());
    }
    return "Success";
}
bool GL::Program::Status(){
    GLint status;
    GetProgramiv(mHandle, GL_LINK_STATUS, &status);
    return (status == GL_TRUE);
}
GLint GL::Program::GetAttribute(std::string name){
    return GetAttribLocation(mHandle, name.c_str());
}
void GL::Program::Uniform1f(const std::string &attrib, float f){
	UseProgram(mHandle);
	GLint attribLoc = GetUniformLocation(mHandle, attrib.c_str());
	GL::Uniform1f(attribLoc, f);
}
void GL::Program::Uniform3fv(const std::string &attrib, const glm::vec3 &vec){
	UseProgram(mHandle);
	GLint attribLoc = GetUniformLocation(mHandle, attrib.c_str());
	GL::Uniform3fv(attribLoc, 1, glm::value_ptr(vec));
}
void GL::Program::Uniform4fv(const std::string &attrib, const glm::vec4 &vec){
    UseProgram(mHandle);
    GLint attribLoc = GetUniformLocation(mHandle, attrib.c_str());
    GL::Uniform4fv(attribLoc, 1, glm::value_ptr(vec));
}
void GL::Program::UniformMat4x4(const std::string &attrib, const glm::mat4 &matrix){
    UseProgram(mHandle);
    GLint attribLoc = GetUniformLocation(mHandle, attrib.c_str());
    GL::UniformMatrix4fv(attribLoc, 1, GL_FALSE, glm::value_ptr(matrix));
}
GL::Program::operator GLuint(){
    return mHandle;
}
