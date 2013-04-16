#include <string>
#include <fstream>
#include <vector>
#include <regex>
#include <map>
#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SOIL.h>
#include "glfunctions.h"
#include "util.h"

std::ostream& operator<<(std::ostream &os, const glm::vec2 &v){
	for (size_t i = 0; i < v.length(); ++i)
		os << v[i] << ", ";
	return os;
}
std::ostream& operator<<(std::ostream &os, const glm::vec3 &v){
	for (size_t i = 0; i < v.length(); ++i)
		os << v[i] << ", ";
	return os;
}
std::string Util::ReadFile(const std::string &file){
    std::string content = "";
    std::ifstream fileIn(file.c_str());
    if (fileIn.is_open()){
        content = std::string((std::istreambuf_iterator<char>(fileIn)),
            std::istreambuf_iterator<char>());
    }
    return content;
}
void Util::LoadObj(const std::string &file, std::vector<glm::vec3> &verts, 
		std::vector<unsigned short> &indices)
{
	if (file.substr(file.length() - 3, 3) != "obj"){
		std::cout << "substr: " << file.substr(file.length() - 4, 3) << std::endl;
		std::cout << "Only .obj files are supported, passed: " << file << std::endl;
		return;
	}
	std::ifstream objFile(file);
	if (!objFile.is_open()){
		std::cout << "Failed to find obj file: " << file << std::endl;
		return;
	}
	//Matches a float value ie. of form xx.xx
	std::regex matchNum("((\\+|-)?[0-9]+)((\\.[0-9]+)?)");
	//Matches a face triangle vertex info
	std::regex matchVert("([0-9]+)/([0-9]+)/([0-9]+)");

	/*
	* The method for doing this will be to parse in all the vertex
	* positions, texture coords and normals then use the face information
	* to generate unique vertices with the v/vt/vn information when needed
	* and simultaneously generate the index buffer to point to the face vertices
	* ie. if we encounter a vertex we've already stored, 5/1/1 & we hit 5/1/1
	* again, instead of saving another vertex we add the existing index to the buffer
	* TODO: Handle simple mtl files. I think this will require creating a Model class of some
	* sort since now we're going to be dealing with a lot more information,
	* ie. vertices + indices + textures + ambient/diffuse/spec info and etc.
	*/
	//Store the vertex information as we read it in
	std::vector<glm::vec3> tempVert, tempNorm;
	std::vector<glm::vec2> tempTex;
	std::map<std::string, unsigned short> vertIndices;

	//Read the file and apply appropriate regexes to lines to get data
	std::string line = "";
	while (std::getline(objFile, line)){
		//Parse vertex info, points, texture coords and normals
		if (line.at(0) == 'v'){
			//Matching vertices
			if (line.at(1) == ' '){
				capture(line, tempVert);
			}
			//Match texture coords
			if (line.at(1) == 't'){
				capture(line, tempTex);
			}
			//Match texture coords
			if (line.at(1) == 'n')
				capture(line, tempNorm);
		}
		//Match faces
		if (line.at(0) == 'f'){
			auto begin = std::sregex_iterator(line.begin(), line.end(), matchVert);
			auto end = std::sregex_iterator();
			//Check if the vertex is already stored, if it is push back the index,
			//if not store the vertex and push in the new index
			for (std::sregex_iterator i = begin; i != end; ++i){
				std::map<std::string, unsigned short>::iterator fnd = vertIndices.find(i->str());
				if (fnd != vertIndices.end())
					indices.push_back(fnd->second);
				//If we don't find it we need to store the vertex info, add to the indices vector and 
				//associate the face with the index in the map
				else {
					std::vector<int> v;
					capture(i->str(), matchNum, v);
					//Setup the interleaved vbo
					verts.push_back(tempVert[v[0] - 1]);
					verts.push_back(tempNorm[v[2] - 1]);
					verts.push_back(glm::vec3(tempTex[v[1] - 1], 0));
					//Store the new index
					//We need to divide the index by the number of items we're interleaving
					indices.push_back((verts.size() - 1) / 3);
					//Associate the face with the index
					vertIndices[i->str()] = indices.back();
				}
			}
		}
		//Load material
		if (line.substr(0, 3) == "mtl"){
			//The mtllib is specified like: mtllib name.mtl, so we find the first space
			//then split string at that and take the second half
			size_t pos = line.find_first_of(' ');
			std::string mtl = line.substr(pos + 1);
			std::cout << "this model will use mtllib:" << mtl << std::endl;
			std::map<std::string, Material> mats;
			LoadMaterials("../res/" + mtl, mats);
		}
	}
}
void Util::LoadMaterials(const std::string &file, std::map<std::string, Material> &mats){
	if (file.substr(file.length() - 3, 3) != "mtl"){
		std::cout << "substr: " << file.substr(file.length() - 4, 3) << std::endl;
		std::cout << "Only .mtl files are supported, passed: " << file << std::endl;
		return;
	}
	std::ifstream mtlFile(file);
	if (!mtlFile.is_open()){
		std::cout << "Failed to find mtl file: " << file << std::endl;
		return;
	}
	//For matching numbers
	std::regex matchNum("((\\+|-)?[0-9]+)((\\.[0-9]+)?)");

	///Run through the file until we encounter a material definition, "newmtl"
	std::string line;
	while (std::getline(mtlFile, line)){
		//When we encounter a material definition load the material and store it in the map
		//We stop reading the material when we encounter a blank line or another material defintion
		if (line.substr(0, 6) == "newmtl"){
			Material material;
			//read the name
			material.name = line.substr(line.find_first_of(' ') + 1);
			std::cout << "reading mat name:" << material.name << std::endl;
			while (std::getline(mtlFile, line) && !line.empty() && line.substr(0, 6) != "newmtl"){
				if (line.substr(0, 2) == "Ka")
					std::cout << "ambient color: " << line.substr(2) << std::endl;
				else if (line.substr(0, 2) == "Kd")
					std::cout << "diffuse color: " << line.substr(2) << std::endl;
				else if (line.substr(0, 2) == "Ks")
					std::cout << "specular color: " << line.substr(2) << std::endl;
				else if (line.substr(0, 2) == "Ns")
					std::cout << "specular exponent: " << line.substr(2) << std::endl;
			}
		}
	}
}
GLuint Util::LoadTexture(const std::string &file){
	return SOIL_load_OGL_texture(file.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB);
}
void Util::capture(const std::string &str, std::vector<glm::vec2> &vect, const std::regex &reg){
	auto begin = std::sregex_iterator(str.begin(), str.end(), reg);
	auto end = std::sregex_iterator();
	size_t idx = 0;
	glm::vec2 v;
	for (std::sregex_iterator i = begin; i != end, idx < v.length(); ++i, ++idx)
		v[idx] = lexicalCast<float>(i->str());

	vect.push_back(v);
}
void Util::capture(const std::string &str, std::vector<glm::vec3> &vect, const std::regex &reg){
	auto begin = std::sregex_iterator(str.begin(), str.end(), reg);
	auto end = std::sregex_iterator();
	size_t idx = 0;
	glm::vec3 v;
	for (std::sregex_iterator i = begin; i != end, idx < v.length(); ++i, ++idx)
		v[idx] = lexicalCast<float>(i->str());

	vect.push_back(v);
}