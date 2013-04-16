#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "glcamera.h"

GL::Camera::Camera(const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up)
	: mEye(eye), mCenter(center), mUp(up)
{
	Refresh();
}
void GL::Camera::Zoom(float amt){
	//Compute the view direction vector, and move the eye and center along that
	//direction by the desired amount
	glm::vec3 viewDir = ViewDir();
	mEye += amt * viewDir;
	mCenter += amt * viewDir;
	Refresh();
}
void GL::Camera::Pitch(float deg){
	//determine the horizontal axis from the cross product of up and view
	glm::vec3 horizontal = glm::normalize(glm::cross(ViewDir(), mUp));
	glm::mat4 rotation = glm::rotate(deg, horizontal);
	glm::vec4 viewDir = rotation * glm::vec4(ViewDir(), 0);
	glm::vec3 dir(viewDir);
	//adjust the scene center
	mCenter = mEye + glm::normalize(dir);
	//We also need to rotate the up vector
	mUp = glm::normalize(glm::vec3(rotation * glm::vec4(mUp, 0)));
	Refresh();
}
void GL::Camera::Roll(float deg){
	//Rotate the up vector about the view direction by some degrees
	mUp = glm::normalize(glm::vec3(glm::rotate(deg, ViewDir()) * glm::vec4(mUp, 0)));
	Refresh();
}
void GL::Camera::Yaw(float deg){
	//TODO: Is this correct? I think it is
	//Get the look vector, rotate it about the up vector by deg and 
	//find a new center point that is one away from eye, along the look vector
	glm::vec4 viewDir = glm::rotate(deg, mUp) * glm::vec4(ViewDir(), 0);
	glm::vec3 dir(viewDir);
	//Adjust the scene center
	mCenter = mEye + glm::normalize(dir);
	Refresh();
}
void GL::Camera::Strafe(const glm::vec3 &v){
	//Should change it to rotate the vector into the model rotation
	//so if we strafe on x, ie. L/R we'll always strafe L/R no matter how we've rotated
	mEye += v;
	mCenter += v;
	Refresh();
}
const glm::mat4& GL::Camera::View(){
	mChanged = false;
	return mView;
}
glm::vec3 GL::Camera::ViewDir() const {
	return glm::normalize(mCenter - mEye);
}
bool GL::Camera::Changed() const {
	return mChanged;
}
void GL::Camera::Refresh(){
	mView = glm::lookAt(mEye, mCenter, mUp);
	mChanged = true;
}
