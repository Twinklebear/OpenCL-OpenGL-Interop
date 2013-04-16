#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <map>
#include <vector>
#include <glm/glm.hpp>
#include "glvertexarray.h"
#include "glprogram.h"
#include "material.h"

/**
* A basic model that can be drawn, the model will specify
* the VAO, the ambient, diffuse and specular colors and/or textures
* in the form of the associated Materials along with
* its model transformation matrix, however the program
* to use won't be associated with the model
* maybe I will associate the program, it'll be easier
*/
class Model {
public:
	/**
	* Create a model using the vertices and indices from the vectors
	* passed in, this basic mode will simply draw a gray model
	* @param verts The model vertices interleaved like: vec3(pos), vec3(normal), vec3(texCoord)
	* @param indices The indices for the element buffer
	*/
	Model(const std::vector<glm::vec3> &verts, const std::vector<unsigned short> &indices);
	/**
	* Add a material for usage by the model
	* @param name Name to associate with the material
	* @param mat The material
	*/
	void AddMaterial(const std::string &name, Material &mat);
	/**
	* Assign the shader program to be used for drawing
	* @param prog The program to use
	*/
	void UseProgram(GL::Program &prog);
	/**
	* Specify which material should be used when drawing the model
	* @param name Name of material to use
	*/
	void UseMaterial(const std::string &name);
	/**
	* Translate the model by some amount from its current position
	* @param vect The vector describing the amount to translate by
	*/
	void Translate(const glm::vec3 &vect);
	/**
	* Rotate the model by some amount from its current rotation along
	* some axis
	* @param angle Angle in degrees to rotate by
	* @param axis Axis to rotate around
	*/
	void Rotate(float angle, const glm::vec3 &axis);
	/**
	* Scale the model by some amount from its current scaling
	* @param scale The scale vector to use containg the x, y, z scale information
	*/
	void Scale(const glm::vec3 &scale);
	///Draw the model
	void Draw();

private:
	///The model transformation matrix
	glm::mat4 mModel;
	///The model's VAO
	GL::VertexArray mVao;
	///The shader program we're using
	GL::Program mProg;
	///Materials associated with the model
	std::map<std::string, Material> mMaterials;
	///The active material
	Material *mActiveMat;
};

#endif