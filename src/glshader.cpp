#include <string>
#include <fstream>
#include <functional>
#include <memory>
#include <vector>
#include <SDL_opengl.h>
#include "glfunctions.h"
#include "util.h"
#include "glshader.h"

const std::function<void(GLuint*)> GL::Shader::sShaderDelete =
	[](GLuint *i){ GL::DeleteShader(*i); };

GL::Shader::Shader(std::string file, TYPE type) : mType(type) {
    Create(file);
}
std::string GL::Shader::Debug(){
    if (!Status()){
        //Get the log length and then get the log
        GLint logLength;
        GetShaderiv(mHandle, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength);
        GetShaderInfoLog(mHandle, logLength, NULL, &log[0]);

        //Setup our error string
        std::string errorMsg = "";
        switch (mType){
        case TYPE::FRAGMENT:
            errorMsg = "Fragment shader error log:\n";
            break;
        case TYPE::VERTEX:
            errorMsg = "Vertex shader error log:\n";
            break;
        case TYPE::GEOMETRY:
            errorMsg = "Geometry shader error log:\n";
            break;
        }
        //Construct and return log message
        errorMsg += std::string(log.begin(), log.end());
        return errorMsg;
    }
    return "Success";
}
bool GL::Shader::Status(){
    GLint status;
    GetShaderiv(mHandle, GL_COMPILE_STATUS, &status);
    return (status == GL_TRUE);
}
void GL::Shader::Create(const std::string &file){
	mHandle = Handle(GL::CreateShader(mType), sShaderDelete);
    std::string src = Util::ReadFile(file);
    const char *srcPtr = src.c_str();
    GL::ShaderSource(mHandle, 1, &srcPtr, NULL);
    GL::CompileShader(mHandle);
}
GL::Shader::operator GLuint(){
    return mHandle;
}
