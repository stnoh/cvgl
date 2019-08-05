#ifndef APP_GL_EXAMPLE_202
#define APP_GL_EXAMPLE_202

#include <cvgl/AppGLBase.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <opencv2/highgui.hpp>

#include <cvgl/GLCamera.h>
#include <cvgl/Kinect2.h>

class Example202 : public AppGLBase
{
public:
	Example202(const int width, const int height) : AppGLBase(width, height) {};

	// mandatory member function
	void Draw(const int width, const int height);

	// override member functions
	bool Init();
	void End();

	bool Update();

private:
	void drawView3D(glm::mat4 proj, glm::mat4 view);

	// member objects & variables
	glm::fvec4 BgColor[2] = { glm::fvec4(0.5, 0.5, 0.5, 1.0), glm::fvec4(0.2, 0.3, 0.5, 1.0) };

	// global viewer
	glm::quat GlobalViewRotation;
	glm::vec3 GlobalViewPosition;
	void resetGlobalView() {
		GlobalViewPosition = glm::vec3(0.75f, 0.50f, 1.25f);
		GlobalViewRotation = glm::quat(glm::radians(glm::vec3(45.0f, -37.5f, -30.0f)));
	}

	// local viewer
	enum RawImage { INVALID = -1, COLOR, DEPTH, IR } imgType = RawImage::IR;
	bool show_raw_image  = true;
	bool show_pointcloud = true;

	// RGB-D camera & camera objects
	cvgl::Kinect2  *kinect2 = nullptr;
	cvgl::GLCamera *depthCamGL = nullptr;
};

#endif