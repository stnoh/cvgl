#include "Example201.h"
#include <cvgl/DrawGL2D.h>
#include <cvgl/DrawGL3D.h>


///////////////////////////////////////////////////////////////////////////////
// mandatory member function
///////////////////////////////////////////////////////////////////////////////
void Example201::Draw(const int width, const int height)
{
	// left / right
	int w = width / 2;

	cv::Mat img;
	double scale = 255.0 / 1200.0;

	cv::Size imgSize = cv::Size(w, height);
	switch (imgType) {
	case cvgl::RawImage::COLOR: img = camera->GetColorImage(); imgSize = camera->GetColorImage().size(); scale = 1.0; break;
	case cvgl::RawImage::DEPTH: img = camera->GetDepthImage(); imgSize = camera->GetDepthImage().size(); break;
	case cvgl::RawImage::IR   : img = camera->GetIrImage();    imgSize = camera->GetIrImage().size();    break;
	default: break;
	}
	float aspect_inv = (float)imgSize.height / (float)imgSize.width;
	int h = (int)(aspect_inv * (float)w);

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
			// right side: local viewer
			glDisable(GL_DEPTH_TEST);
			if (show_raw_image) cvgl::drawImage(img, viewport[id], scale);
			glEnable(GL_DEPTH_TEST);

			// 3D point cloud in local view
			glm::mat4 proj = (cvgl::RawImage::COLOR == imgType) ? glCameraRig->GetProjMatrix(1) : glCameraRig->GetProjMatrix(0);
			glm::mat4 view = (cvgl::RawImage::COLOR == imgType) ? glCameraRig->GetViewMatrix(1) : glCameraRig->GetViewMatrix(0);
			drawView3D(proj, view);
			glDisable(GL_DEPTH_TEST);
		}
	}
	glDisable(GL_SCISSOR_TEST);
}
void Example201::drawView3D(glm::mat4 proj, glm::mat4 view)
{
	glMatrixMode(GL_PROJECTION); glLoadMatrixf(glm::value_ptr(proj));
	glMatrixMode(GL_MODELVIEW);  glLoadMatrixf(glm::value_ptr(view));

	// draw color/depth camera together
	glCameraRig->DrawCameraRig();

	// draw point cloud from depth image
	if (show_pointcloud) {
		glm::mat4 depth_view = glCameraRig->GetViewMatrix(0);
		glm::mat4 depth_pose = glm::inverse(depth_view);

		glPushMatrix();
		glMultMatrixf(glm::value_ptr(depth_pose));

		std::vector<glm::vec3>   points;
		std::vector<glm::u8vec3> colors;
		camera->GetPointCloud(points, colors);
		cvgl::drawPointCloud(points, colors);

		glPopMatrix();
	}
}


///////////////////////////////////////////////////////////////////////////////
// override member functions
///////////////////////////////////////////////////////////////////////////////
bool Example201::Init()
{
	// initialize camera object
	camera = new cvgl::RSCamera();

	// read calibration data (if exists)
#ifdef SHARED_DATA_PATH
	char calib_data[1024];
	sprintf_s(calib_data, "%s/calib_rgbd.xml", SHARED_DATA_PATH);
	camera->ReadCalibrationData(calib_data);
#endif
	// instantiate color/depth camera rig
	glCameraRig = new cvgl::GLCameraRig();
	glCameraRig->Init(camera);

	camera->InitLive();

	// initialize global viewer
	resetGlobalView();

	////////////////////////////////////////////////////////////
	// UI: AntTweakBar
	////////////////////////////////////////////////////////////

	// global viewer (3D point cloud)
#if 1
	TwAddButton(bar, "Global-init", [](void *client) {
		Example201* _this = (Example201*)client; _this->resetGlobalView();
	}, this, "group='Global' label='init' ");
	TwAddVarRW(bar, "Global-rot", TwType::TW_TYPE_QUAT4F, &GlobalViewRotation, "group='Global' label='rot'  open");
	TwAddVarRW(bar, "Global-posX", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.x, "group='Global' label='posX' step=0.01");
	TwAddVarRW(bar, "Global-posY", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.y, "group='Global' label='posY' step=0.01");
	TwAddVarRW(bar, "Global-posZ", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.z, "group='Global' label='posZ' step=0.01");
#endif

	// local viewer (2D image)
	std::vector<TwEnumVal> imageTypeEV = { { cvgl::RawImage::INVALID, "<EMPTY>" },
		{ cvgl::RawImage::COLOR, "Color" },
		{ cvgl::RawImage::DEPTH, "Depth" },
		{ cvgl::RawImage::IR   , "Infrared" } };
	TwType twImageType = TwDefineEnum("Raw image", &imageTypeEV[0], (unsigned int)imageTypeEV.size());
	TwAddVarRW(bar, "Raw image", twImageType, &imgType, NULL);

	TwAddVarRW(bar, "Show raw image" , TW_TYPE_BOOLCPP, &show_raw_image , NULL);
	TwAddVarRW(bar, "Show pointcloud", TW_TYPE_BOOLCPP, &show_pointcloud, NULL);

	return true;
}
void Example201::End()
{
	// remove camera object
	if (camera) {
		camera->End();
		delete camera;
	}

	if (glCameraRig) delete glCameraRig;
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
