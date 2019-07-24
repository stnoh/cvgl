#ifndef APP_GL_EXAMPLE_102
#define APP_GL_EXAMPLE_102

#include "cvgl/AppGLBase.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "cvgl/GLCamera.h"

class AppGL : public AppGLBase
{
public:
	AppGL(const int width, const int height) : AppGLBase(width, height) {};

	void Draw(const int width, const int height);

	// override member functions
	bool Init();
	void End();

private:

	void drawView3D(glm::mat4 proj, glm::mat4 view);

	// member objects & variables
	glm::fvec4 BgColor[2] = { glm::fvec4(0.5, 0.5, 0.5, 1.0), glm::fvec4(0.2, 0.3, 0.5, 1.0) };

	glm::quat GlobalViewRotation;
	glm::vec3 GlobalViewPosition;
	void resetGlobalView() {
		GlobalViewPosition = glm::vec3(0.0f, 3.0f, 30.0f);
		GlobalViewRotation = glm::quat(glm::radians(glm::vec3(45.0f, -37.5f, -30.0f)));
	}

	cvgl::GLCamera *rendercam = nullptr;
	void resetCameraView() {
		// oblique view as default
		rendercam->position = glm::vec3(0.0f, 10.0f, 10.0f);
		rendercam->rotation = glm::quat(glm::radians(glm::vec3(-45.0f, 0.0f, 0.0f)));
	}
};

#endif
