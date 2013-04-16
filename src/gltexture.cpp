#include <string>
#include <functional>
#include <SDL_opengl.h>
#include "handle.h"
#include "util.h"
#include "gltexture.h"

const std::function<void(GLuint*)> GL::Texture::sTexDeleter = 
	[](GLuint *i){ glDeleteTextures(1, i); };

GL::Texture::Texture(){
}
GL::Texture::Texture(const std::string &file){
	Load(file);
}
void GL::Texture::Load(const std::string &file){
	mHandle = Handle(Util::LoadTexture(file), sTexDeleter);
}
GL::Texture::operator GLuint(){
	return mHandle;
}
