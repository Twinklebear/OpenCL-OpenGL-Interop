#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <regex>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <SDL_opengl.h>
#include "material.h"

//Functions for reading in and printing out glm vectors, should be free of any namespace
//restrictions
//Debugging: add ability to print glm::vec2 and 3
std::ostream& operator<<(std::ostream &os, const glm::vec2 &v);
std::ostream& operator<<(std::ostream &os, const glm::vec3 &v);

namespace Util {
    //Read a file and return its contents as a string
	std::string ReadFile(const std::string &file);
	/**
	* Load the object file passed into the verts vector, data will be interleaved and stored as 
	* glm::vec3, in the order of position, normal, uv, with indices for the element buffer
	* being placed in the indices vector
	* @param file File to load from
	* @param verts Vector to fill with vertex data
	* @param indices Vector to fill with the index data
	*/
	void LoadObj(const std::string &file, std::vector<glm::vec3> &verts, 
		std::vector<unsigned short> &indices);
	/**
	* Load the materials defined in a material lib file. Materials will be placed into
	* the map passed indexed by their name
	* @param file The mtl file to load from
	* @param mats The map to store the materials in
	*/
	void LoadMaterials(const std::string &file, std::map<std::string, Material> &mats);
	/**
	* Load a texture and return the handle
	* TODO: Add ability to set options for loading the texture
	* @param file File to load texture from
	* @return The GLuint associated with the texture
	*/
	GLuint LoadTexture(const std::string &file);
	//Cast a string to a desired type and return it
	template<class T>
	T lexicalCast(const std::string &str){
		std::stringstream ss;
		T res;
		ss << str;
		ss >> res;
		return res;
	}
	//Using scanf in our lexicalCast float/int gets us some more speed
	template<>
	inline float lexicalCast(const std::string &str){
		float f;
		sscanf(str.c_str(), "%f", &f);
		return f;
	}
	template<>
	inline int lexicalCast(const std::string &str){
		int i;
		sscanf(str.c_str(), "%d", &i);
		return i;
	}
	//Capture matched values from regex results, values will be pushed onto vector
	template<class T>
	void capture(const std::string &str, const std::regex &reg, std::vector<T> &vect){
		auto begin = std::sregex_iterator(str.begin(), str.end(), reg);
		auto end = std::sregex_iterator();
		for (std::sregex_iterator i = begin; i != end; ++i)
			vect.push_back(lexicalCast<T>(i->str()));
	}
	//Capture matched floats for filling a glm::vec2
	void capture(const std::string &str, std::vector<glm::vec2> &vect,
		const std::regex &reg = std::regex("((\\+|-)?[0-9]+)((\\.[0-9]+)?)"));
	//Capture matched floats for filling a glm::vec3
	void capture(const std::string &str, std::vector<glm::vec3> &vect,
		const std::regex &reg = std::regex("((\\+|-)?[0-9]+)((\\.[0-9]+)?)"));
	//Capture a glm::vec3
}

#endif