#include "Example203.h"
#include "KinectFusionHelperEx.h"


///////////////////////////////////////////////////////////////////////////////
// mandatory member function
///////////////////////////////////////////////////////////////////////////////
void Example203::Draw(const int width, const int height)
{
	// left / right
	int w = width / 2;
	int h = height;

	// viewport settings ( 0:left / 1:right )
	cv::Rect viewport[2] = {
		cv::Rect(0, 0, w, height),
		cv::Rect(w, 0, w, h)
	};

	glEnable(GL_SCISSOR_TEST); // to cut left/right viewport
	for (int id = 0; id < 2; id++)
	{
		glViewport(viewport[id].x, viewport[id].y, viewport[id].width, viewport[id].height);
		glScissor (viewport[id].x, viewport[id].y, viewport[id].width, viewport[id].height);

		glClearColor(BgColor[id].r, BgColor[id].g, BgColor[id].b, BgColor[id].a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (0 == id) {
			// left side: global viewer
			glm::mat4 proj = glm::infinitePerspective(glm::radians(53.1301f), (float)viewport[id].width / (float)viewport[id].height, 0.1f);
			glm::mat4 view = glm::translate(glm::mat4(1.0f), -GlobalViewPosition);
			view = view * glm::toMat4(GlobalViewRotation);

			glEnable(GL_DEPTH_TEST);
			drawView3D(proj, view);
			glDisable(GL_DEPTH_TEST);
		}
		else {
			// [TODO]
		}
	}
	glDisable(GL_SCISSOR_TEST);
}
void Example203::drawView3D(glm::mat4 proj, glm::mat4 view)
{
	// [TODO] draw view-dependent point cloud from KinectFusion
}


///////////////////////////////////////////////////////////////////////////////
// override member functions
///////////////////////////////////////////////////////////////////////////////
bool Example203::Init()
{
	// [TODO] initialize KinectFusion offline mode object

	// initialize global viewer
	resetGlobalView();

	////////////////////////////////////////////////////////////
	// UI: AntTweakBar
	////////////////////////////////////////////////////////////

	// global viewer
#if 1
	TwAddButton(bar, "Global-init", [](void *client) {
		Example203* _this = (Example203*)client; _this->resetGlobalView();
	}, this, "group='Global' label='init' ");
	TwAddVarRW(bar, "Global-rot", TwType::TW_TYPE_QUAT4F, &GlobalViewRotation, "group='Global' label='rot'  open");
	TwAddVarRW(bar, "Global-posX", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.x, "group='Global' label='posX' step=0.01");
	TwAddVarRW(bar, "Global-posY", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.y, "group='Global' label='posY' step=0.01");
	TwAddVarRW(bar, "Global-posZ", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.z, "group='Global' label='posZ' step=0.01");
#endif

	return true;
}
void Example203::End()
{
	// [TODO] finalize KinectFusion offline mode object
}

bool Example203::Update()
{
	// [TODO] update KinectFusion offline mode object

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	Example203 app(1024, 512);

	app.SetInternalProcess(true);
	app.run();

	app.End();
	return EXIT_SUCCESS;
}
