#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "glvertexarray.h"
#include "glprogram.h"
#include "material.h"
#include "model.h"

Model::Model(const std::vector<glm::vec3> &verts, const std::vector<unsigned short> &indices)
	: mActiveMat(nullptr)
{
	mVao.Reference(GL::VertexBuffer(verts), "vbo");
	mVao.ElementBuffer(indices);
	//What to set as the attribute pointers here? Should
	//I just do layout location stuff?
}
void Model::AddMaterial(const std::string &name, Material &mat){
	mMaterials[name] = mat;
}
void Model::UseProgram(GL::Program &prog){
	mProg = prog;
	//Setup attributes
	mVao.SetAttribPointer("vbo", mProg.GetAttribute("position"), 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), 0);
	mVao.SetAttribPointer("vbo", mProg.GetAttribute("normal"), 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), 
		(void*)sizeof(glm::vec3));
	mVao.SetAttribPointer("vbo", mProg.GetAttribute("texIn"), 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3),
		(void*)(sizeof(glm::vec3) * 2));

	//Setup uniforms
	mProg.UniformMat4x4("m", mModel);
	//If a material is selected pass that info as well
	if (mActiveMat != nullptr){
		mProg.Uniform3fv("kA", mActiveMat->kA);
		mProg.Uniform3fv("kD", mActiveMat->kD);
		mProg.Uniform3fv("kS", mActiveMat->kS);
		mProg.Uniform1f("nS", mActiveMat->nS);
	}
}
void Model::UseMaterial(const std::string &name){
	std::map<std::string, Material>::iterator mat = mMaterials.find(name);
	if (mat == mMaterials.end()){
		//Throw runtime err?
		std::cout << "Invalid material name: " << name << std::endl;
		return;
	}
	mActiveMat = &(mat->second);
	//Set the program uniform info
	mProg.Uniform3fv("kA", mActiveMat->kA);
	mProg.Uniform3fv("kD", mActiveMat->kD);
	mProg.Uniform3fv("kS", mActiveMat->kS);
	mProg.Uniform1f("nS", mActiveMat->nS);
}
void Model::Translate(const glm::vec3 &vect){
	mModel = glm::translate(vect) * mModel;
	mProg.UniformMat4x4("m", mModel);
}
void Model::Rotate(float angle, const glm::vec3 &axis){
	mModel = mModel * glm::rotate(angle, axis);
	mProg.UniformMat4x4("m", mModel);
}
void Model::Scale(const glm::vec3 &scale){
	//The order of operations with rotate scale may be an issue
}
void Model::Draw(){
	GL::UseProgram(mProg);
	GL::BindVertexArray(mVao);
	glDrawElements(GL_TRIANGLES, mVao.NumElements("elem"), GL_UNSIGNED_SHORT, 0);
}
