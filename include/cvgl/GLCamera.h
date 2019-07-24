/******************************************************************************
GL camera class that manages projection and modelview matrices
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#ifndef GL_CAMERA
#define GL_CAMERA

#include <GL/glew.h>
#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace cvgl {

class GLCamera
{
public:

	////////////////////////////////////////////////////////////
	// intrinsic (FOV): projection matrix
	////////////////////////////////////////////////////////////
	const glm::mat4 GetProjMatrix();

	// set projection matrix by normalized camera matrix [0.0:1.0]
	void SetCameraMatrixCV(std::vector<float> camParams4x1, float z_near);
	void SetCameraMatrixCV(std::vector<float> camParams4x1, float z_near, float z_far);

	// default values for camera frustum
	bool is_ortho = false;
	bool is_inf   = true;

	float z_near =   0.1f;
	float z_far  = 100.0f;
	float L = -0.5f;
	float R = +0.5f;
	float B = -0.5f;
	float T = +0.5f;

	////////////////////////////////////////////////////////////
	// extrinsic (RT): rotation and translation
	////////////////////////////////////////////////////////////
	const glm::mat4 GetViewMatrix() { return glm::inverse(GetPoseMatrix()); }
	const glm::mat4 GetPoseMatrix()
	{
		glm::mat4 T = glm::translate(position);
		glm::mat4 R = glm::toMat4(rotation);
		return T * R;
	}

	// extrinsic (RT)
	glm::vec3 position;
	glm::quat rotation;
};

}

#endif
