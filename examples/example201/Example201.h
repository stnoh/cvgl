#ifndef APP_GL_EXAMPLE_201
#define APP_GL_EXAMPLE_201

#include <cvgl/AppGLBase.h>

#include <glm/glm.hpp>
#include <opencv2/highgui.hpp>

#include <cvgl/RSCamera.h>

class Example201 : public AppGLBase
{
public:
	Example201(const int width, const int height) : AppGLBase(width, height) {};

	// mandatory member function
	void Draw(const int width, const int height);

	// override member functions
	bool Init();
	void End();

	bool Update();

private:

	// member objects & variables
	glm::fvec4 BgColor[2] = { glm::fvec4(0.5, 0.5, 0.5, 1.0), glm::fvec4(0.2, 0.3, 0.5, 1.0) };

	cvgl::RSCamera *camera = nullptr;
};

#endif
