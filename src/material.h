#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include <glm/glm.hpp>
#include "gltexture.h"

/**
* A simple class to describe an object's material as described
* in a .mtl file under newmtl
*/
struct Material {
	Material();
	/**
	* Construct a simple un-textured material
	* @param name The material name
	* @param kA Ambient color
	* @param kD Diffuse color
	* @param kS Specular color
	* @param nS Specular highlight exponent
	*/
	Material(std::string name, glm::vec3 kA, glm::vec3 kD, glm::vec3 kS, float nS);
	/**
	* Construct a texture material, no bump map or nS map support yet
	* @param name The material name
	* @param kA Ambient color
	* @param kD Diffuse color
	* @param kS Specular color
	* @param nS Specular highlight exponent
	* @param mapKa Ambient texture 
	* @param mapKd Diffuse texture
	* @param mapKs Specular texture
	*/
	Material(std::string name, glm::vec3 kA, glm::vec3 kD, glm::vec3 kS, float nS,
		GL::Texture mapKa, GL::Texture mapKd, GL::Texture mapKs);

	std::string name;
	glm::vec3 kA, kD, kS;
	float nS;
	GL::Texture mapKa, mapKd, mapKs;
};

#endif