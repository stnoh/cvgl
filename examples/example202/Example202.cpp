#include "Example202.h"
#include <cvgl/DrawGL2D.h>
#include <cvgl/DrawGL3D.h>


///////////////////////////////////////////////////////////////////////////////
// mandatory member function
///////////////////////////////////////////////////////////////////////////////
void Example202::Draw(const int width, const int height)
{
	// left / right
	int w = width / 2;

	// [TODO]

	// viewport settings ( 0:left / 1:right )
	cv::Rect viewport[2] = {
		cv::Rect(0, 0, w, height),
		cv::Rect(w, 0, w, height)
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
			// right side: local viewer
			glDisable(GL_DEPTH_TEST);
			// [TODO] draw image
			glEnable(GL_DEPTH_TEST);

			// 3D point cloud in local view
			glm::mat4 proj = (COLOR == imgType) ? colorCamGL->GetProjMatrix() : depthCamGL->GetProjMatrix();
			glm::mat4 view = (COLOR == imgType) ? colorCamGL->GetViewMatrix() : depthCamGL->GetViewMatrix();
			drawView3D(proj, view);
			glDisable(GL_DEPTH_TEST);
		}
	}
	glDisable(GL_SCISSOR_TEST);
}
void Example202::drawView3D(glm::mat4 proj, glm::mat4 view)
{
	glMatrixMode(GL_PROJECTION); glLoadMatrixf(glm::value_ptr(proj));
	glMatrixMode(GL_MODELVIEW);  glLoadMatrixf(glm::value_ptr(view));

	// color camera
	glColor3f(1.0f, 0.0f, 0.0f);
	colorCamGL->SetCameraCoord([](){}, true);

	// depth camera
	glColor3f(1.0f, 1.0f, 0.0f);
	depthCamGL->SetCameraCoord([&](){
		// [TODO] point cloud
		}, true);
}


///////////////////////////////////////////////////////////////////////////////
// override member functions
///////////////////////////////////////////////////////////////////////////////
bool Example202::Init()
{
	// [TODO] initialize camera object

	// color camera
	colorCamGL = new cvgl::GLCamera();
	{
		// [TODO] set color camera matrix
	}

	// depth camera: 15[cm] - 1.2[m]
	depthCamGL = new cvgl::GLCamera();
	{
		// [TODO] set depth camera matrix
	}

	// initialize global viewer
	resetGlobalView();

	////////////////////////////////////////////////////////////
	// UI: AntTweakBar
	////////////////////////////////////////////////////////////

	// global viewer (3D point cloud)
#if 1
	TwAddButton(bar, "Global-init", [](void *client) {
		Example202* _this = (Example202*)client; _this->resetGlobalView();
	}, this, "group='Global' label='init' ");
	TwAddVarRW(bar, "Global-rot", TwType::TW_TYPE_QUAT4F, &GlobalViewRotation, "group='Global' label='rot'  open");
	TwAddVarRW(bar, "Global-posX", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.x, "group='Global' label='posX' step=0.01");
	TwAddVarRW(bar, "Global-posY", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.y, "group='Global' label='posY' step=0.01");
	TwAddVarRW(bar, "Global-posZ", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.z, "group='Global' label='posZ' step=0.01");
#endif

	// local viewer (2D image)
	std::vector<TwEnumVal> imageTypeEV = { { RawImage::INVALID, "<EMPTY>" },
		{ RawImage::COLOR, "Color" },
		{ RawImage::DEPTH, "Depth" },
		{ RawImage::IR   , "Infrared" } };
	TwType twImageType = TwDefineEnum("Raw image", &imageTypeEV[0], (unsigned int)imageTypeEV.size());
	TwAddVarRW(bar, "Raw image", twImageType, &imgType, NULL);

	TwAddVarRW(bar, "Show raw image" , TW_TYPE_BOOLCPP, &show_raw_image , NULL);
	TwAddVarRW(bar, "Show pointcloud", TW_TYPE_BOOLCPP, &show_pointcloud, NULL);

	return true;
}
void Example202::End()
{
	// [TODO] remove camera object

	if (colorCamGL) delete colorCamGL;
	if (depthCamGL) delete depthCamGL;
}

bool Example202::Update()
{
	// [TODO] update camera object

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	Example202 app(1280, 480);

	app.SetInternalProcess(true); // always update for RGBD camera
	app.run();

	app.End();
	return EXIT_SUCCESS;
}
