#ifndef APP_GL_EXAMPLE_203
#define APP_GL_EXAMPLE_203

#include <cvgl/AppGLBase.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <opencv2/highgui.hpp>

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
};

#endif
