/******************************************************************************
RGB-D camera rig by GLCamera
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include "cvgl/GLCamera.h"
#include "cvgl/DrawGL3D.h"
#include <opencv2/core/core.hpp>

using namespace cvgl;

///////////////////////////////////////////////////////////////////////////////
// ctor & dtor
///////////////////////////////////////////////////////////////////////////////
GLCameraRig::GLCameraRig()
{
}
GLCameraRig::~GLCameraRig()
{
	if (glColorCam) delete glColorCam;
	if (glDepthCam) delete glDepthCam;
}


///////////////////////////////////////////////////////////////////////////////
// initialization
///////////////////////////////////////////////////////////////////////////////
bool GLCameraRig::Init(const char* calibXmlFile)
{
	cv::FileStorage fs(calibXmlFile, cv::FileStorage::READ);

	if (!fs.isOpened()) return false;

	cv::Size color_imageSize, depth_imageSize;
	float color_focalLength, depth_focalLength;
	unsigned short depth_clipping_near;
	unsigned short depth_clipping_far;

	cv::Mat rMat;
	cv::Vec3f tVec;

	try {
		fs["color_image_size"   ] >> color_imageSize;
		fs["color_camera_focal" ] >> color_focalLength;

		fs["depth_image_size"   ] >> depth_imageSize;
		fs["depth_camera_focal" ] >> depth_focalLength;
		fs["depth_clipping_near"] >> depth_clipping_near;
		fs["depth_clipping_far" ] >> depth_clipping_far;

		fs["depth_to_color_rmat"] >> rMat;
		fs["depth_to_color_tvec"] >> tVec;
	}
	catch (const cv::Exception e)
	{
		fprintf(stderr, "ERROR: read XML.\n");
		return false;
	}

	// set intrinsic parameters for GLCamera
	std::vector<float> color_camParams4x1(4);
	color_camParams4x1[0] = color_focalLength / (float)color_imageSize.width;
	color_camParams4x1[1] = color_focalLength / (float)color_imageSize.height;
	color_camParams4x1[2] = 0.5f;
	color_camParams4x1[3] = 0.5f;

	// set intrinsic parameters for GLCamera
	std::vector<float> depth_camParams4x1(4);
	depth_camParams4x1[0] = depth_focalLength / (float)depth_imageSize.width;
	depth_camParams4x1[1] = depth_focalLength / (float)depth_imageSize.height;
	depth_camParams4x1[2] = 0.5f;
	depth_camParams4x1[3] = 0.5f;

	const float mm2m = 0.001f;
	float z_near = mm2m * depth_clipping_near;
	float z_far  = mm2m * depth_clipping_far;

	// GLCamera for visualization
	glColorCam = new GLCamera();
	glColorCam->SetCameraMatrixCV(color_camParams4x1, z_near - 0.05f); // slightly nearer than depth cam
	glDepthCam = new GLCamera();
	glDepthCam->SetCameraMatrixCV(depth_camParams4x1, z_near, z_far); // valid depth [0.2:1.2] [m]

	// view matrix (GL) from camera RT (CV)
	glm::mat4 view_GL = glm::mat4(1.0f);
	for (int j = 0; j < 3; j++)
		for (int i = 0; i < 3; i++) {
			view_GL[j][i] = rMat.at<double>(i, j);
		}

	view_GL[0][1] = -view_GL[0][1];
	view_GL[0][2] = -view_GL[0][2];
	view_GL[1][0] = -view_GL[1][0];
	view_GL[2][0] = -view_GL[2][0];

	view_GL[3][0] = +tVec[0];
	view_GL[3][1] = -tVec[1];
	view_GL[3][2] = -tVec[2];

	// set initial relative pose
	Color2Depth = view_GL;
	Depth2Color = glm::inverse(view_GL); // Depth->Color = inv(Color->Depth)

	glm::mat4 initialPose = glm::mat4();
	glColorCam->SetPoseMatrixGL(initialPose);
	glDepthCam->SetPoseMatrixGL(initialPose * Color2Depth);

	return true;
}
bool GLCameraRig::Init(RGBDCamera* camera)
{
	// get calibration data from this instance
	std::vector<float> camParamsColor, camParamsDepth;
	std::vector<float> distCoeffColor, distCoeffDepth;
	glm::mat4 camPoseColor, camPoseDepth;
	camera->GetCalibData(0, camParamsDepth, camPoseDepth); // id #0: depth at origin
	camera->GetCalibData(1, camParamsColor, camPoseColor); // id #1

	// GLCamera for visualization
	glColorCam = new GLCamera();
	glColorCam->SetCameraMatrixCV(camParamsColor, 0.15f); // slightly nearer than depth cam
	glDepthCam = new GLCamera();
	glDepthCam->SetCameraMatrixCV(camParamsDepth, 0.2f, 1.2f); // valid depth [0.2:1.2] [m]

	// set initial relative pose
	Depth2Color = camPoseColor;
	Color2Depth = glm::inverse(camPoseColor); // Color->Depth = inv(Depth->Color)

	glm::mat4 initialPose = glm::mat4();
	glColorCam->SetPoseMatrixGL(initialPose);
	glDepthCam->SetPoseMatrixGL(initialPose * Color2Depth);

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// main routine
///////////////////////////////////////////////////////////////////////////////
void GLCameraRig::SetCameraPoseGL(int id, glm::mat4 cam_pose_GL)
{
	if (1 == id) {
		glColorCam->SetPoseMatrixGL(cam_pose_GL);
		glDepthCam->SetPoseMatrixGL(cam_pose_GL * Color2Depth);
	}
	else {
		glDepthCam->SetPoseMatrixGL(cam_pose_GL);
		glColorCam->SetPoseMatrixGL(cam_pose_GL * Depth2Color);
	}
}
void GLCameraRig::DrawCameraRig()
{
	auto DrawGLCamera = [](GLCamera* cam) {
		glm::mat4 camProj = cam->GetProjMatrix();
		glm::mat4 camPose = cam->GetPoseMatrix();
		glPushMatrix();
		glMultMatrixf(glm::value_ptr(camPose));
		drawCameraFrustum(camProj);
		glPopMatrix();
	};

	glColor3f(0.0f, 1.0f, 1.0f); DrawGLCamera(glDepthCam);
	glColor3f(1.0f, 1.0f, 0.0f); DrawGLCamera(glColorCam);
}
