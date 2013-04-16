#include <string>
#include <stdexcept>
#include <memory>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>
#include <SDL_syswm.h>
#include "glfunctions.h"
#include "glvertexarray.h"
#include "glprogram.h"
#include "window.h"

#include <iostream>

Window::Window(std::string title, int width, int height)
	: mWindow(nullptr, SDL_DestroyWindow)
{
    //Setup our window
    mBox.x = 0;
    mBox.y = 0;
    mBox.w = width;
    mBox.h = height;

	//Setup SDL_GL attributes we want, this must be done before opening window/making context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	//Turn on multisample
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    //Create our window
    mWindow.reset(SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        mBox.w, mBox.h, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL));
    //Make sure it created ok
    if (mWindow == nullptr)
        throw std::runtime_error("Failed to create window");

    //Get the glcontext
    mContext = SDL_GL_CreateContext(mWindow.get());
	//Setup some properites for the context
    glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    std::cout << "OpenGL Info:\n"
		<< "OpenGL Version: " << glGetString(GL_VERSION) << std::endl
        << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl
        << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl
        << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n\n";
    MakeCurrent();
    GL::SetupGLFunctions();
}
Window::~Window(){
	Close();
}
void Window::Init(){
    //initialize all SDL subsystems
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
		throw std::runtime_error("SDL Init Failed");
	IMG_Init(IMG_INIT_PNG);
}
void Window::Quit(){
    SDL_Quit();
}
void Window::Draw(GL::VertexArray &vao, GL::Program &p, GLenum mode, int first, size_t count){
    GL::UseProgram(p);
    GL::BindVertexArray(vao);
    glDrawArrays(mode, first, count);
}
void Window::DrawElements(GL::VertexArray &vao, GL::Program &p, GLenum mode, int count, size_t offset){
	GL::UseProgram(p);
	GL::BindVertexArray(vao);
	glDrawElements(mode, count, GL_UNSIGNED_SHORT, (void*)offset);
}
void Window::DrawElementsTextured(GL::VertexArray &vao, GL::Program &p, GL::Texture &tex, GLenum mode, int count, size_t offset){
	GL::UseProgram(p);
	GL::ActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	GL::BindVertexArray(vao);
	glDrawElements(mode, count, GL_UNSIGNED_SHORT, (void*)offset);
}
void Window::Clear(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void Window::Present(){
	glFlush();
    SDL_GL_SwapWindow(mWindow.get());
}
SDL_Rect Window::Box(){
    return mBox;
}
bool Window::MakeCurrent(){
	return (SDL_GL_MakeCurrent(mWindow.get(), mContext) == 0);
}
void Window::Close(){
	SDL_GL_DeleteContext(mContext);
	mWindow.~unique_ptr();
}
