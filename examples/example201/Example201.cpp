#include "Example201.h"
#include <cvgl/DrawGL3D.h>

#include <cvgl/PopupWindow.h>


///////////////////////////////////////////////////////////////////////////////
// mandatory member function
///////////////////////////////////////////////////////////////////////////////
void Example201::Draw(const int width, const int height)
{
	// left / right
	int w = width / 2;

	// viewport settings ( 0:left / 1:right )
	cv::Rect viewport[2] = {
		cv::Rect(0, 0, w, height),
		cv::Rect(w, 0, w, height),
	};

	glEnable(GL_SCISSOR_TEST); // to cut left/right viewport
	for (int id = 0; id < 2; id++)
	{
		glViewport(viewport[id].x, viewport[id].y, viewport[id].width, viewport[id].height);
		glScissor(viewport[id].x, viewport[id].y, viewport[id].width, viewport[id].height);

		glClearColor(BgColor[id].r, BgColor[id].g, BgColor[id].b, BgColor[id].a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (0 == id) {
			// [TODO] draw depth camera image (?)
		}
		else {
			// [TODO] draw color camera image (?)
		}
	}
	glDisable(GL_SCISSOR_TEST);
}


///////////////////////////////////////////////////////////////////////////////
// override member functions
///////////////////////////////////////////////////////////////////////////////
bool Example201::Init()
{
	// [TODO] initialize camera object here

	resetGlobalView();

	////////////////////////////////////////////////////////////
	// UI: AntTweakBar
	////////////////////////////////////////////////////////////
	TwDefine("Bar size='250 550'");

	return true;
}
void Example201::End()
{
	// [TODO] remove camera object here
}

bool Example201::Update()
{
	// [TODO] update camera object here

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	Example201 app(960, 540);

	app.SetInternalProcess(true); // always update for RGBD camera
	app.run();

	app.End();
	return EXIT_SUCCESS;
}
