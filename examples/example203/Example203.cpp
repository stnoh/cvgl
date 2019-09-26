#include "Example203.h"
#include <cvgl/PopupWindow.h>

#include <cvgl/ConvertCVGL.h>
#include <cvgl/DrawGL3D.h>


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
			drawView3D(proj, view_matrix[0], 0);
		}
		else {
			glm::mat4 proj = glCamera->GetProjMatrix();
			drawView3D(proj, view_matrix[1], 1);
		}
	}
	glDisable(GL_SCISSOR_TEST);
}
void Example203::drawView3D(glm::mat4 proj, glm::mat4 view, int id)
{
	////////////////////////////////////////////////////////////
	// GL immediate mode (~ 1.x)
	////////////////////////////////////////////////////////////
	glMatrixMode(GL_PROJECTION); glLoadIdentity(); glMultMatrixf(glm::value_ptr(proj));
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	cvgl::setLight(GL_LIGHT0, LightDir); // (view-independent lighting)
	glMultMatrixf(glm::value_ptr(view));

	// camera frustum
	if (show_camera) {
		glPushMatrix();
		glMultMatrixf(glm::value_ptr(glCamera->GetPoseMatrix()));
		cvgl::drawCameraFrustum(glCamera->GetProjMatrix());
		glPopMatrix();
	}

	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	//cvgl::drawAxes(0.1f); // 0.1 [m] at origin

	// draw boundary box of volume
	if (show_volume_box) {
		glPushMatrix();
		glMultMatrixf(glm::value_ptr(volume_pose));

		glColor3f(1.0f, 1.0f, 1.0f);
		cvgl::drawAxes(0.1f);
		cvgl::drawAABB(minAB, maxAB);

		glPopMatrix();
	}

	glDisable(GL_LIGHTING);

	////////////////////////////////////////////////////////////
	// draw point cloud of view
	////////////////////////////////////////////////////////////
	if (show_point_cloud)
	{
		glm::mat4 pose = glm::inverse(view);

		glPushMatrix();
		glLoadIdentity();

		kinfu->RenderVolume(id, view, points[id], normals[id], colors[id]);
		colors[id] = cvgl::GetNormalColors(normals[id]);
		cvgl::drawPointCloud(points[id], colors[id]);

		glPopMatrix();
	}

	////////////////////////////////////////////////////////////
	// 3D mesh (in world coordinate)
	////////////////////////////////////////////////////////////
	if (show_tri_mesh) {
		glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
		glPolygonMode(GL_FRONT, GL_FILL);
		cvgl::drawTriMesh(V, N, C, F);
	}

	glDisable(GL_RESCALE_NORMAL);
	glDisable(GL_DEPTH_TEST);
}


///////////////////////////////////////////////////////////////////////////////
// override member functions
///////////////////////////////////////////////////////////////////////////////
bool Example203::Init()
{
	// initialize KinectFusion offline mode object
	kinfu = new KinectFusionGL();

	// synchronize GLCamera and KinectFusion camera info
	std::vector<float> camParams = { 1.0f, 1.0f, 0.5f, 0.5f };
	glCamera = new cvgl::GLCamera();
	glCamera->SetCameraMatrixCV(camParams, 0.01f);
	cv::Size imageSize = cv::Size(640, 480);
	kinfu->InitCamera(camParams, imageSize, 0.01f, 1.20f);

	// initialize viewer
	reconsParams = kinfu->GetKinfuParams();
	kinfu->GetVolumeArea(volume_pose, minAB, maxAB);

	resetGlobalView();


	////////////////////////////////////////////////////////////
	// UI: AntTweakBar
	////////////////////////////////////////////////////////////

	// load XML file to import scanned TSDF data
	TwAddButton(bar, "LoadXML", [](void* client) { ((Example203*)client)->LoadXML(); }, this, " group='File' key=F1 ");
	TwAddVarRO(bar, "VPM", TwType::TW_TYPE_FLOAT, &reconsParams.voxelsPerMeter, " group='File' ");
	TwAddVarRO(bar, "voxelsX", TwType::TW_TYPE_UINT32, &reconsParams.voxelCountX, " group='File' ");
	TwAddVarRO(bar, "voxelsY", TwType::TW_TYPE_UINT32, &reconsParams.voxelCountY, " group='File' ");
	TwAddVarRO(bar, "voxelsZ", TwType::TW_TYPE_UINT32, &reconsParams.voxelCountZ, " group='File' ");

	// setting for global view
#if 1
	TwAddButton(bar, "Global-init", [](void *client) {
		Example203* _this = (Example203*)client; _this->resetGlobalView();
	}, this, "group='Global' label='init' ");
	TwAddVarRW(bar, "Global-rot", TwType::TW_TYPE_QUAT4F, &GlobalViewRotation  , "group='Global' label='rot' ");
	TwAddVarRW(bar, "Global-posX", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.x, "group='Global' label='posX' step=0.01");
	TwAddVarRW(bar, "Global-posY", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.y, "group='Global' label='posY' step=0.01");
	TwAddVarRW(bar, "Global-posZ", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.z, "group='Global' label='posZ' step=0.01");
#endif

	// setting for local view
#if 1
	TwAddButton(bar, "Local-init", [](void *client) {
		Example203* _this = (Example203*)client;
		_this->glCamera->rotation = glm::quat();
		_this->glCamera->position = glm::vec3();
	}, this, "group='Local' label='init' ");
	TwAddVarRW(bar, "Local-rot", TwType::TW_TYPE_QUAT4F, &glCamera->rotation, " group='Local' label='rot' ");
	TwAddVarRW(bar, "Local-posX", TwType::TW_TYPE_FLOAT, &glCamera->position.x, " group='Local' label='posX' step=0.01 ");
	TwAddVarRW(bar, "Local-posY", TwType::TW_TYPE_FLOAT, &glCamera->position.y, " group='Local' label='posY' step=0.01 ");
	TwAddVarRW(bar, "Local-posZ", TwType::TW_TYPE_FLOAT, &glCamera->position.z, " group='Local' label='posZ' step=0.01 ");
#endif

	// setting toggles
	TwAddVarRW(bar, "Vis-Camera"    , TwType::TW_TYPE_BOOLCPP, &show_camera     , " group='Vis' label='Camera' ");
	TwAddVarRW(bar, "Vis-VolumeBox" , TwType::TW_TYPE_BOOLCPP, &show_volume_box , " group='Vis' label='VolumeBox' ");
	TwAddVarRW(bar, "Vis-PointCloud", TwType::TW_TYPE_BOOLCPP, &show_point_cloud, " group='Vis' label='PointCloud' ");
	TwAddVarRW(bar, "Vis-TriMesh"   , TwType::TW_TYPE_BOOLCPP, &show_tri_mesh   , " group='Vis' label='TriMesh' ");

	return true;
}
void Example203::End()
{
	if (kinfu) delete kinfu;
	if (glCamera) delete glCamera;
}
bool Example203::Update()
{
	////////////////////////////////////////////////////////////
	// update view matrix
	// global (id:0) / local (id:1) view
	////////////////////////////////////////////////////////////
	glm::mat4 view = glm::translate(glm::mat4(1.0f), -GlobalViewPosition);
	view_matrix[0] = view * glm::toMat4(GlobalViewRotation);
	view_matrix[1] = glCamera->GetViewMatrix();

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////
void Example203::LoadXML()
{
	std::string filepath;
	if (cvgl::OpenFileWindow(filepath, "XML file (*.xml)\0*.xml\0\0"s)) {
		if (kinfu->LoadVolume(filepath.c_str())) {
			glfwSetWindowTitle(window, filepath.c_str());

			kinfu->GetVolumeArea(volume_pose, minAB, maxAB);
			kinfu->GetTriMesh(V, N, C, F);
			reconsParams = kinfu->GetKinfuParams();

			return;
		}

		// clear window title when error occurs
		glfwSetWindowTitle(window, "");
	}
}


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	Example203 app(1600, 800);

	app.SetInternalProcess(true);
	app.run();

	app.End();
	return EXIT_SUCCESS;
}
