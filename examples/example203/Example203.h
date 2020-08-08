#ifndef APP_GL_EXAMPLE_203
#define APP_GL_EXAMPLE_203

#include <cvgl/AppGLBase.h>
#include <cvgl/GLCamera.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <opencv2/highgui.hpp>

#include "KinectFusionGL.h"

class Example203 : public AppGLBase
{
public:
	Example203(const int width, const int height) : AppGLBase(width, height) {};

	// mandatory member function
	void Draw(const int width, const int height);

	// override member functions
	bool Init();
	void End();
	bool Update();

	// file I/O
	void LoadXML();
	void LoadImages();

	void ExtractMesh();

private:
	void drawView3D(glm::mat4 proj, glm::mat4 view, const int id);
	glm::vec4 LightDir = glm::vec4(-1, -1, -1, 0);

	// member objects & variables
	glm::fvec4 BgColor[2] = { glm::fvec4(0.5, 0.5, 0.5, 1.0), glm::fvec4(0.2, 0.3, 0.5, 1.0) };

	// 3D behaviour
	KinectFusionGL    *kinfu       = nullptr;

	// 3D volume by KinectFusion
	NUI_FUSION_RECONSTRUCTION_PARAMETERS reconsParams;
	glm::mat4 volume_pose;
	glm::vec3 minAB, maxAB;

	// extracted 3D mesh
	std::vector<glm::vec3> V, N;
	std::vector<glm::u8vec3> C;
	std::vector<GLuint> F;

	// depth image as point cloud
	glm::mat4 view_matrix[2];
	std::vector<glm::vec3>   points[2], normals[2];
	std::vector<glm::u8vec3> colors[2];

	// registered 2d images
	std::vector<glm::mat4> registered_cameras_pose;
	std::vector<cv::Mat>   registered_cameras_img;

	////////////////////////////////////////////////////////////
	// AntTweakBar control
	////////////////////////////////////////////////////////////

	// global viewer
	glm::quat GlobalViewRotation;
	glm::vec3 GlobalViewPosition;
	void resetGlobalView() {
		GlobalViewPosition = glm::vec3(0.0f, 0.0f, 0.75f);
		GlobalViewRotation = glm::quat(glm::radians(glm::vec3(-55.0f, +35.0f, +20.0f)));
	}

	// toggles
	bool show_raw_image   = true;
	bool show_camera      = true;
	bool show_volume_box  = true;
	bool show_point_cloud = true;
	bool show_tri_mesh    = false;

	// control on registered 2d images
	cvgl::GLCameraRig *glCameraRig = nullptr;

	cvgl::RawImage imgType = cvgl::RawImage::COLOR;

	void changeLocalView(int _view_num) {
		if (_view_num < 0 || registered_cameras_count <= _view_num) {
			current_view_num = -1;
			fprintf(stderr, "ERROR: invalid # of view.\n");
			return;
		}

		current_view_num = _view_num;
		glm::mat4 pose = registered_cameras_pose[current_view_num];
		//printf("set to view #%d\n", current_view_num);

		glCameraRig->SetCameraPoseGL(1, pose);
		glCameraRig->GetViewMatrix(0);
	}
	int registered_cameras_count = -1;
	int current_view_num = -1;
};

#endif
