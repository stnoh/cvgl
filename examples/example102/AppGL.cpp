#include "AppGL.h"
#include "cvgl/DrawGL3D.h"

#include <opencv2/opencv.hpp>


///////////////////////////////////////////////////////////////////////////////
// mandatory
///////////////////////////////////////////////////////////////////////////////
void AppGL::Draw(const int width, const int height)
{
	// left / right
	int w = width / 2;

	// viewport settings ( 0:left / 1:right )
	cv::Rect viewport[2] = {
		cv::Rect(0, 0, w, height),
		cv::Rect(w, height-w, w, w) // same aspect
	};

	glEnable(GL_SCISSOR_TEST); // to cut left/right viewport
	for (int id = 0; id < 2; id++)
	{
		glViewport(viewport[id].x, viewport[id].y, viewport[id].width, viewport[id].height);
		glScissor (viewport[id].x, viewport[id].y, viewport[id].width, viewport[id].height);

		glClearColor(BgColor[id].r, BgColor[id].g, BgColor[id].b, BgColor[id].a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 proj = rendercam->GetProjMatrix();
		glm::mat4 view = rendercam->GetViewMatrix();
		
		// for global viewer (id:0)
		if (0 == id) {
			proj = glm::infinitePerspective(glm::radians(GlobalViewFoV), (float)viewport[id].width / (float)viewport[id].height, 0.1f);
			view = glm::translate(glm::mat4(1.0f), -GlobalViewPosition);
			view = view * glm::toMat4(GlobalViewRotation);
		}

		glEnable(GL_DEPTH_TEST);
		drawView3D(proj, view);
		glDisable(GL_DEPTH_TEST);
	}
	glDisable(GL_SCISSOR_TEST);
}

void AppGL::drawView3D(glm::mat4 proj, glm::mat4 view)
{
	glMatrixMode(GL_PROJECTION); glLoadMatrixf(glm::value_ptr(proj));
	glMatrixMode(GL_MODELVIEW);  glLoadMatrixf(glm::value_ptr(view));

	glm::mat4 cam_proj = rendercam->GetProjMatrix();
	glm::mat4 cam_pose = rendercam->GetPoseMatrix();

	// draw 3D axes
	glEnable(GL_LIGHTING);
	cvgl::setLight(GL_LIGHT0, glm::vec4(-1.0f, -1.0f, -1.0f, 0.0f));
	cvgl::drawAxes(1.0f);
	{
		glPushMatrix(); glMultMatrixf(glm::value_ptr(cam_pose));
		cvgl::drawAxes(1.0f);
		glPopMatrix();
	}

	glDisable(GL_LIGHTING);

	// draw 3D grid
	glLineWidth(1.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	cvgl::drawGridXZ(20.0, 20);

	// draw camera frustum
	glLineWidth(2.0f);
	glColor3f(1.0f, 1.0f, 0.0f);
	{
		glPushMatrix();
		glMultMatrixf(glm::value_ptr(cam_pose));
		cvgl::drawCameraFrustum(cam_proj);
		glPopMatrix();
	}
}


///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////
bool AppGL::Init()
{
	// instantiate camera
	rendercam = new cvgl::GLCamera();
	rendercam->SetCameraMatrixCV({ 1.0f, 1.0f, 0.5f, 0.5f }, 0.1f, 100.0f);

	resetGlobalView();
	resetCameraView();

	////////////////////////////////////////////////////////////
	// UI: AntTweakBar
	////////////////////////////////////////////////////////////
	TwDefine("Bar size='250 550'");

	// global viewer
#if 1
	TwAddButton(bar, "Global-init", [](void *client) {
		AppGL* _this = (AppGL*)client; _this->resetGlobalView();
	}, this, "group='Global' label='init' ");
	TwAddVarRW(bar, "Global-rot" , TwType::TW_TYPE_QUAT4F, &GlobalViewRotation , "group='Global' label='rot'  open");
	TwAddVarRW(bar, "Global-posX", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.x, "group='Global' label='posX' step=0.01");
	TwAddVarRW(bar, "Global-posY", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.y, "group='Global' label='posY' step=0.01");
	TwAddVarRW(bar, "Global-posZ", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.z, "group='Global' label='posZ' step=0.01");
#endif

	// local camera viewer
#if 1
	TwAddButton(bar, "Local-init", [](void *client) {
		AppGL* _this = (AppGL*)client; _this->resetCameraView();
	}, this, "group='Local' label='init' ");
	TwAddVarRW(bar, "Local-rot" , TwType::TW_TYPE_QUAT4F, &rendercam->rotation  , "group='Local' label='rot'  open");
	TwAddVarRW(bar, "Local-posX", TwType::TW_TYPE_FLOAT , &rendercam->position.x, "group='Local' label='posX' step=0.01");
	TwAddVarRW(bar, "Local-posY", TwType::TW_TYPE_FLOAT , &rendercam->position.y, "group='Local' label='posY' step=0.01");
	TwAddVarRW(bar, "Local-posZ", TwType::TW_TYPE_FLOAT , &rendercam->position.z, "group='Local' label='posZ' step=0.01");

	TwAddVarRW(bar, "Camera-ortho"   , TwType::TW_TYPE_BOOLCPP, &rendercam->is_ortho, "group='Camera' label='ortho'");
	TwAddVarRW(bar, "Camera-infinity", TwType::TW_TYPE_BOOLCPP, &rendercam->is_inf  , "group='Camera' label='infinity'");
	TwAddVarRW(bar, "Camera-z_near"  , TwType::TW_TYPE_FLOAT  , &rendercam->z_near  , "group='Camera' label='z_near' step=0.01");
	TwAddVarRW(bar, "Camera-z_far"   , TwType::TW_TYPE_FLOAT  , &rendercam->z_far   , "group='Camera' label='z_far'  step=0.01");
#endif

	return true;
}
void AppGL::End()
{
	delete rendercam;
}


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	AppGL app(1280, 640);

	app.SetInternalProcess(true);
	app.run();

	app.End();
	return EXIT_SUCCESS;
}
