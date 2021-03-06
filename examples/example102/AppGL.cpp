#include "AppGL.h"
#include <cvgl/DrawGL3D.h>
#include <cvgl/ConvertCVGL.h>

#include <cvgl/plyFileIO.h>
#include <cvgl/PopupWindow.h>


///////////////////////////////////////////////////////////////////////////////
// mandatory member function
///////////////////////////////////////////////////////////////////////////////
void AppGL::Draw(const int width, const int height)
{
	// left / right
	int w = width / 2;

	// viewport settings ( 0:left / 1:right )
	cv::Rect viewport[2] = {
		cv::Rect(0, 0, w, height),
		cv::Rect(w, height-w, w, w) // keep the same aspect (1:1)
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
			proj = glm::infinitePerspective(glm::radians(53.1301f), (float)viewport[id].width / (float)viewport[id].height, 0.1f);
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

	// draw 3D axes (with lighting)
	glEnable(GL_LIGHTING);
	glEnable(GL_RESCALE_NORMAL);

	cvgl::setLight(GL_LIGHT0, glm::vec4(-1.0f, -1.0f, -1.0f, 0.0f));
	cvgl::drawAxes(1.0f);
	{
		glPushMatrix(); glMultMatrixf(glm::value_ptr(cam_pose));
		cvgl::drawAxes(1.0f);
		glPopMatrix();

		// draw 3D mesh (in model space)
		glm::mat4 model = glm::translate(ModelPosition) * glm::mat4(ModelRotation);
		model = glm::scale(model, glm::vec3(ModelUniScale));
		{
			glPushMatrix(); glMultMatrixf(glm::value_ptr(model));
			cvgl::drawTriMesh(V, N, C, F);
			glPopMatrix();
		}
	}

	glDisable(GL_LIGHTING);
	glDisable(GL_RESCALE_NORMAL);

	// draw 3D grid
	glLineWidth(1.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	cvgl::drawGridXZ(20.0, 20);

	////////////////////////////////////////////////////////////
	// camera space
	////////////////////////////////////////////////////////////
	glColor3f(1.0f, 1.0f, 0.0f);
	{
		glPushMatrix();
		glMultMatrixf(glm::value_ptr(cam_pose));

		// camera frustum
		glLineWidth(2.0f);
		cvgl::drawCameraFrustum(cam_proj);

		// point cloud
		glPointSize(1.0f);
		cvgl::drawPointCloud(points); // single color for consistency
		//cvgl::drawPointCloud(points, colors);

		glPopMatrix();
	}
}


///////////////////////////////////////////////////////////////////////////////
// override member functions
///////////////////////////////////////////////////////////////////////////////
bool AppGL::Init()
{
	// instantiate camera object
	rendercam = new cvgl::GLCamera();
	rendercam->SetCameraMatrixCV({ 1.0f, 1.0f, 0.5f, 0.5f }, 1.0f, 30.0f);
	rendercam->is_inf = true;

	resetGlobalView();
	resetCameraView();
	resetModelMatrix();

	// instantiate frame buffer object
	offscreenFBO = new cvgl::FBO();

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

	// model coordinate
#if 1
	TwAddButton(bar, "Model-LoadMesh",
		[](void* client) {
		AppGL* _this = (AppGL*)client;
		_this->LoadMesh();
	}, this, "group='Model' label='LoadMesh' key=F3");

	TwAddButton(bar, "Model-init", [](void *client) {
		AppGL* _this = (AppGL*)client; _this->resetModelMatrix();
	}, this, "group='Model' label='init' ");
	TwAddVarRW(bar, "Model-rot"  , TwType::TW_TYPE_QUAT4F, &ModelRotation  , "group='Model' label='rot'   open");
	TwAddVarRW(bar, "Model-posX" , TwType::TW_TYPE_FLOAT , &ModelPosition.x, "group='Model' label='posX'  step=0.01");
	TwAddVarRW(bar, "Model-posY" , TwType::TW_TYPE_FLOAT , &ModelPosition.y, "group='Model' label='posY'  step=0.01");
	TwAddVarRW(bar, "Model-posZ" , TwType::TW_TYPE_FLOAT , &ModelPosition.z, "group='Model' label='posZ'  step=0.01");
	TwAddVarRW(bar, "Model-scale", TwType::TW_TYPE_FLOAT , &ModelUniScale  , "group='Model' label='scale' step=0.01");
#endif

	// view matrix related properties (local camera)
#if 1
	TwAddButton(bar, "Local-init", [](void *client) {
		AppGL* _this = (AppGL*)client; _this->resetCameraView();
	}, this, "group='Local' label='init' ");
	TwAddVarRW(bar, "Local-rot" , TwType::TW_TYPE_QUAT4F, &rendercam->rotation  , "group='Local' label='rot'  open");
	TwAddVarRW(bar, "Local-posX", TwType::TW_TYPE_FLOAT , &rendercam->position.x, "group='Local' label='posX' step=0.01");
	TwAddVarRW(bar, "Local-posY", TwType::TW_TYPE_FLOAT , &rendercam->position.y, "group='Local' label='posY' step=0.01");
	TwAddVarRW(bar, "Local-posZ", TwType::TW_TYPE_FLOAT , &rendercam->position.z, "group='Local' label='posZ' step=0.01");

	TwDefine("Bar/Local opened=false"); // close group as default
#endif

	// point cloud from rendered depth image
#if 1
	TwAddButton(bar, "CreatePointCloud",
		[](void* client) {
			AppGL* _this = (AppGL*)client;
			_this->CreatePointCloud();
		}, this, "key=SPACE");

	TwAddButton(bar, "SavePointCloud",
		[](void* client) {
		AppGL* _this = (AppGL*)client;
		_this->SavePointCloud();
	}, this, "key=F4");
#endif

	// projection matrix related properties
#if 1
	TwAddVarRW(bar, "Camera-ortho"   , TwType::TW_TYPE_BOOLCPP, &rendercam->is_ortho, "group='Camera' label='ortho'");
	TwAddVarRW(bar, "Camera-infinity", TwType::TW_TYPE_BOOLCPP, &rendercam->is_inf  , "group='Camera' label='infinity'");
	TwAddVarRW(bar, "Camera-z_near"  , TwType::TW_TYPE_FLOAT  , &rendercam->z_near  , "group='Camera' label='z_near' step=0.01");
	TwAddVarRW(bar, "Camera-z_far"   , TwType::TW_TYPE_FLOAT  , &rendercam->z_far   , "group='Camera' label='z_far'  step=0.01");

	TwAddVarCB(bar, "Camera-FoV", TwType::TW_TYPE_FLOAT, 
		[](const void* value, void* obj) {
			cvgl::GLCamera* rendercam = (cvgl::GLCamera*)obj;
			rendercam->SetCameraFoV(*(float*)value, 1.0f, 1.0f, rendercam->z_near);
		},
		[](void* value, void* obj) {
			cvgl::GLCamera* rendercam = (cvgl::GLCamera*)obj;
			*(float*)value = rendercam->GetCameraFoV();
		},
		rendercam, "group='Camera' label='FoV' min=1.0 max=179.0");

	TwAddVarRW(bar, "Camera-L", TwType::TW_TYPE_FLOAT, &rendercam->L, "group='Camera' label='Left'   step=0.01");
	TwAddVarRW(bar, "Camera-R", TwType::TW_TYPE_FLOAT, &rendercam->R, "group='Camera' label='Right'  step=0.01");
	TwAddVarRW(bar, "Camera-B", TwType::TW_TYPE_FLOAT, &rendercam->B, "group='Camera' label='Bottom' step=0.01");
	TwAddVarRW(bar, "Camera-T", TwType::TW_TYPE_FLOAT, &rendercam->T, "group='Camera' label='Top'    step=0.01");

	TwDefine("Bar/Camera opened=false"); // close group as default
#endif

	return true;
}
void AppGL::End()
{
	if (rendercam   ) delete rendercam;
	if (offscreenFBO) delete offscreenFBO;
}


///////////////////////////////////////////////////////////////////////////////
// user-defined function
///////////////////////////////////////////////////////////////////////////////
void AppGL::CreatePointCloud()
{
	// clear point cloud in advance
	points.clear();
	colors.clear();

	////////////////////////////////////////
	// offscreen rendering to generate data
	////////////////////////////////////////
	int w = width / 2;
	int h = w;

	offscreenFBO->Resize(w, h);
	offscreenFBO->Enable();

	// draw 3D scene
	glm::mat4 proj = rendercam->GetProjMatrix();
	glm::mat4 view = rendercam->GetViewMatrix();

	glClearColor(BgColor[1].r, BgColor[1].g, BgColor[1].b, BgColor[1].a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	drawView3D(proj, view);
	glDisable(GL_DEPTH_TEST);

	// get image as cv::Mat from GL buffer
	cv::Mat color = cvgl::GetRenderedColorImage8U(w, h);
	cv::Mat depth = cvgl::GetRenderedDepthImage32F(w, h);

	offscreenFBO->Disable();

	////////////////////////////////////////
	// post-process
	////////////////////////////////////////

	// image sanity check
#if 1
	cv::imshow("color", color);
	cv::imshow("depth", depth);
#endif

	// convert color/depth image pair to point cloud
	cvgl::ConvertColorDepthImage2PointCloud(glm::inverse(proj), color, depth, points, colors);

	return;
}

void AppGL::LoadMesh()
{
	std::string filepath;
	if (cvgl::OpenFileWindow(filepath, "ply file (*.ply)\0*.ply\0"s))
	{
		// load ply mesh
		printf("loading: %s ... ", filepath.c_str());
		cvgl::ReadTriMeshPly(filepath.c_str(), V, N, F);
		printf("[DONE]\n");
		printf("(V, N, F) = (%d, %d, %d)\n", V.size(), N.size(), F.size() / 3);

		// extract normal color from vertex normals
		C = cvgl::GetNormalColors(N);
	}
}
void AppGL::SavePointCloud()
{
	std::string filepath;
	if (cvgl::SaveFileWindow(filepath, "ply file (*.ply)\0*.ply\0"s))
	{
		// add view normal
		std::vector<glm::vec3> normals(points.size(), glm::vec3(0.0f, 0.0f, 1.0f));

		// save ply with view normal
		printf("writing: %s ... ", filepath.c_str());
		cvgl::WritePointCloudPly(filepath.c_str(), points, normals, colors);
		printf("[DONE]\n");
	}
}


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	AppGL app(1280, 640);

	app.SetInternalProcess(false); // update when it is dirty
	app.run();

	app.End();
	return EXIT_SUCCESS;
}
