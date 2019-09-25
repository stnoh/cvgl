/******************************************************************************
GL camera class that manages projection and modelview matrices
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#ifndef GL_CAMERA
#define GL_CAMERA

#include <GL/glew.h>
#include <vector>
#include <functional>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <cvgl/RGBDCamera.h>

namespace cvgl {

class GLCamera
{
public:

	////////////////////////////////////////////////////////////
	// intrinsic (FOV): projection matrix
	////////////////////////////////////////////////////////////
	const glm::mat4 GetProjMatrix();
	
	// get approximated angle FoV
	const float GetCameraFoV(bool isVertical=true)
	{
		if (is_ortho) return FLT_MAX;

		glm::mat4 proj = GetProjMatrix();

		// vertical FoV (FoVy) as default, horizontal FoV (FoVx) if needed
		float value = (isVertical) ? proj[1][1] : proj[0][0];

		float AFoV = 2.0f * glm::degrees( atan2(1.0f, value) );
		return AFoV;
	}

	// set projection matrix by normalized camera matrix [0.0:1.0]
	void SetCameraMatrixCV(const std::vector<float>& camParams4x1, float z_near);
	void SetCameraMatrixCV(const std::vector<float>& camParams4x1, float z_near, float z_far);

	void SetCameraFoV(float FoVy_deg, float width, float height, float z_near, float z_far)
	{
		float FoVy = glm::radians(FoVy_deg);
		glm::mat4 proj = glm::perspectiveFov(FoVy, width, height, z_near, z_far);
		L = (proj[2][0] - 1.0f) / proj[0][0];
		R = (proj[2][0] + 1.0f) / proj[0][0];
		T = (proj[2][1] + 1.0f) / proj[1][1];
		B = (proj[2][1] - 1.0f) / proj[1][1];

		this->z_near = z_near;
		this->z_far  = z_far;
		is_ortho = false;
	}
	void SetCameraFoV(float FoVy, float width, float height, float z_near)
	{
		SetCameraFoV(FoVy, width, height, z_near, z_far);
	}

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
	void SetPoseMatrixGL(const glm::mat4& poseGL)
	{
		rotation = glm::quat(poseGL);
		position = glm::vec3(poseGL[3]);
	}

	// extrinsic (RT)
	glm::vec3 position;
	glm::quat rotation;


	////////////////////////////////////////////////////////////
	// set camera coordinate (=pose matrix)
	////////////////////////////////////////////////////////////
	void SetCameraCoord(std::function<void(void)> func, bool drawFrustum=false);
};

///////////////////////////////////////////////////////////////////////////////
// RGB-D camera rig by GLCamera
// 0: depth camera (origin) / 1: color camera
///////////////////////////////////////////////////////////////////////////////
class GLCameraRig
{
public:
	GLCameraRig();
	virtual ~GLCameraRig();

	bool Init(const char* calibXmlFile);
	bool Init(RGBDCamera* camera);

	void SetCameraPoseGL(int id, glm::mat4 pose);
	glm::mat4 GetProjMatrix(int id)
	{
		return (0 == id) ? glDepthCam->GetProjMatrix() : glColorCam->GetProjMatrix();
	}
	glm::mat4 GetViewMatrix(int id)
	{
		return (0 == id) ? glDepthCam->GetViewMatrix() : glColorCam->GetViewMatrix();
	}

	void DrawCameraRig();

private:
	GLCamera *glColorCam = nullptr;
	GLCamera *glDepthCam = nullptr;

	glm::mat4 Color2Depth;
	glm::mat4 Depth2Color;
};

}

#endif
