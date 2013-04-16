#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include <stdexcept>
#include <memory>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glvertexarray.h"
#include "glprogram.h"
#include "gltexture.h"

/**
*  Window management class, provides a simple wrapper around
*  the SDL_Window and SDL_Renderer functionalities
*/
class Window {
public:
	/**
	* Create a new window and a corresponding gl context and make it current
	* @param title window title
	* @param width window width
	* @param height window height
	*/
	Window(std::string title = "Window", int width = 640, int height = 480);
	//Close the window
	~Window();
    /**
	*  Initialize SDL, setup the window and renderer
	*  @param title The window title
	*/
	static void Init();
	///Quit SDL and destroy the window and renderer
	static void Quit();
    /**
    * Draw a VAO using some program with the desired draw mode
    * @param vao The vao to draw
    * @param p The program to use while drawing
    * @param mode The draw mode to use
    * @param first The first index to begin drawing from
    * @param count The number of entries to draw
    */
    void Draw(GL::VertexArray &vao, GL::Program &p, GLenum mode, int first, size_t count);
	/**
    * Draw a VAO using some program with the desired draw mode using the DrawElements command
	* this requires that the VAO has been associated with an element buffer before calling this function
    * @param vao The vao to draw
    * @param p The program to use while drawing
    * @param mode The draw mode to use
    * @param count The number of entries to draw
	* @param offset Offset in index array to start reading from
    */
	void DrawElements(GL::VertexArray &vao, GL::Program &p, GLenum mode, int count, size_t offset = 0);
	/**
    * Draw a VAO using some program and a texture with the desired draw mode using the DrawElements command
	* this requires that the VAO has been associated with an element buffer before calling this function
    * @param vao The vao to draw
    * @param p The program to use while drawing
	* @param tex The texture to use
    * @param mode The draw mode to use
    * @param count The number of entries to draw
	* @param offset Offset in index array to start reading from
    */
	void DrawElementsTextured(GL::VertexArray &vao, GL::Program &p, GL::Texture &tex, GLenum mode, int count, size_t offset = 0);
    ///Clear the renderer
    void Clear();
    ///Present the renderer, ie. update screen
    void Present();
    ///Get the window's box
    SDL_Rect Box();
	//Make the window the current rendering context
	bool MakeCurrent();
	//Close the window & delete the context
	void Close();

private:
    std::unique_ptr<SDL_Window, void (*)(SDL_Window*)> mWindow;
    SDL_GLContext mContext;
    SDL_Rect mBox;
};

#endif