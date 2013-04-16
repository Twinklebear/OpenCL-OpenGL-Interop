#ifndef GLCAMERA_H
#define GLCAMERA_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace GL {
	/**
	* A simple camera using glm, get the view matrix
	* via View()
	*/
	class Camera {
	public:
		/**
		* Create the camera passing in the eye location, the center point of the scene
		* and the up vector
		* TODO: Should I also make it possible to set a look vector?
		* TODO: Should I use vec4 for eye, center and up and then truncate to vec3 when calling
		*   lookAt?
		* @param eye Camera location
		* @param center Center of scene
		* @param up Up vector for the camera
		*/
		Camera(const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up);
		///Move the camera by some amount along the viewing direction
		void Zoom(float amt);
		///Adjust pitch of camera by some degrees
		void Pitch(float deg);
		///Adjust camera roll by some degrees
		void Roll(float deg);
		///Adjust the yaw of the camera by some degrees
		void Yaw(float deg);
		///Strafe the camera by some vector
		void Strafe(const glm::vec3 &v);
		///Get the viewing transform, also resets changed to false
		const glm::mat4& View();
		//Get the normalized look direction
		glm::vec3 ViewDir() const;
		/**
		* Check if the camera has changed since the last time we got
		* the viewing matrix
		*/
		bool Changed() const;

	private:
		///Recalculate the viewing matrix
		void Refresh();

	private:
		glm::vec3 mEye, mCenter, mUp;
		glm::mat4 mView;
		//Track if the camera has changed
		bool mChanged;
	};
}

#endif