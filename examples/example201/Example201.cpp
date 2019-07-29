#include "Example201.h"
#include <cvgl/DrawGL2D.h>


///////////////////////////////////////////////////////////////////////////////
// mandatory member function
///////////////////////////////////////////////////////////////////////////////
void Example201::Draw(const int width, const int height)
{
	// left / right
	int w = width / 2;

	cv::Size color_size = camera->GetColorImageSize();
	cv::Size depth_size = camera->GetDepthImageSize();
	float color_ratio = (float)color_size.width / (float)color_size.height;
	float depth_ratio = (float)depth_size.width / (float)depth_size.height;

	cv::Mat img[2] = { camera->GetColorImage(), camera->GetIrImage() };

	// viewport settings
	int img_height = glm::min(color_size.height , depth_size.height);
	cv::Rect viewport[2] = {
		cv::Rect(0, 0, w, w / color_ratio), // left : global viewer
		cv::Rect(w, 0, w, w / depth_ratio), // right: selected camera view
	};

	glEnable(GL_SCISSOR_TEST); // to cut left/right viewport
	for (int id = 0; id < 2; id++)
	{
		glViewport(viewport[id].x, viewport[id].y, viewport[id].width, viewport[id].height);
		glScissor (viewport[id].x, viewport[id].y, viewport[id].width, viewport[id].height);

		glClearColor(BgColor[id].r, BgColor[id].g, BgColor[id].b, BgColor[id].a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cvgl::drawImage(img[id], viewport[id]);
	}
	glDisable(GL_SCISSOR_TEST);
}


///////////////////////////////////////////////////////////////////////////////
// override member functions
///////////////////////////////////////////////////////////////////////////////
bool Example201::Init()
{
	// initialize camera object
	camera = new cvgl::RSCamera();
	camera->InitLive();

	////////////////////////////////////////////////////////////
	// UI: AntTweakBar
	////////////////////////////////////////////////////////////
	TwDefine("Bar size='620 100'");

	// [TODO] change image type

	return true;
}
void Example201::End()
{
	// remove camera object
	if (camera) {
		camera->End();
		delete camera;
	}
}

bool Example201::Update()
{
	// update camera object
	camera->Update();

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	Example201 app(1280, 480);

	app.SetInternalProcess(true); // always update for RGBD camera
	app.run();

	app.End();
	return EXIT_SUCCESS;
}
