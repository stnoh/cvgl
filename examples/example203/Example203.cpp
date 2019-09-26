#include "Example203.h"
#include <cvgl/PopupWindow.h>

#include <cvgl/ConvertCVGL.h>
#include <cvgl/DrawGL2D.h>
#include <cvgl/DrawGL3D.h>


///////////////////////////////////////////////////////////////////////////////
// [TEMPORARY] file utility
///////////////////////////////////////////////////////////////////////////////
#include <dirent.h>
#include <sys/stat.h>

std::vector<std::string> GetColorImagesInFolder(std::string folderpath)
{
	// chronical sorting by using std::set<>
	struct FileWithTime {
		time_t time;
		std::string file;
		bool operator < (FileWithTime rhs) const
		{
			if (this->time < rhs.time) return true;
			if (this->time > rhs.time) return false;
			return this->file < rhs.file;
		}
	};
	std::set<FileWithTime> fileWithTime;

	// put as std::set<>
	DIR *dpdf;
	struct dirent *epdf;

	// get "color" images from this directory
	dpdf = opendir(folderpath.c_str());
	if (dpdf != NULL) {
		while (epdf = readdir(dpdf)) {
			std::string file(epdf->d_name);

			struct stat st;
			stat(epdf->d_name, &st);

			if (0 == file.find("color"))
				fileWithTime.insert({ st.st_mtime, file });
		}
	}
	closedir(dpdf);

	// export file list
	std::vector<std::string> files;

	std::set<FileWithTime>::iterator it;
	for (it = fileWithTime.begin(); it != fileWithTime.end(); ++it)
		files.push_back(it->file);

	return files;
}


///////////////////////////////////////////////////////////////////////////////
// mandatory member function
///////////////////////////////////////////////////////////////////////////////
void Example203::Draw(const int width, const int height)
{
	// left / right
	int w = width / 2;
	int h = height;
	
	// 
	float ratio = (ImageType::COLOR == imgType) ? 1920.0/1080.0 : 640.0/480.0 ;

	// viewport settings ( 0:left / 1:right )
	cv::Rect viewport[2];
	viewport[0] = cv::Rect(0, 0, w, height); // global viewer
	viewport[1] = cv::Rect(w, 0, w, w / ratio);

	glEnable(GL_SCISSOR_TEST); // to cut left/right viewport
	for (int id = 0; id < 2; id++)
	{
		glViewport(viewport[id].x, viewport[id].y, viewport[id].width, viewport[id].height);
		glScissor (viewport[id].x, viewport[id].y, viewport[id].width, viewport[id].height);

		glClearColor(BgColor[id].r, BgColor[id].g, BgColor[id].b, BgColor[id].a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// left side: global viewer
		if (0 == id) {
			glm::mat4 proj = glm::infinitePerspective(glm::radians(53.1301f), (float)viewport[id].width / (float)viewport[id].height, 0.1f);
			drawView3D(proj, view_matrix[0], 0);
		}
		// right side: local viewer
		else {
			// show raw color image
			if (COLOR == imgType && show_raw_image)
			{
				if (0 < registered_cameras_count && -1 != current_view_num)
				{
					glDisable(GL_DEPTH_TEST);
					cvgl::drawImage(registered_cameras_img[current_view_num], viewport[id]);
					glEnable(GL_DEPTH_TEST);
				}
			}

			glm::mat4 proj = glCameraRig->GetProjMatrix( (ImageType::COLOR == imgType) ? 1 : 0 );
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

	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_DEPTH_TEST);

	// camera frustum
	if (show_camera) glCameraRig->DrawCameraRig();

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

		//kinfu->RenderVolume(id, view_matrix[id], points[id], normals[id], colors[id]);
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
	glCameraRig = new cvgl::GLCameraRig();

	////////////////////////////////////////////////////////////
	// synchronize GLCamera and KinectFusion camera info
	////////////////////////////////////////////////////////////
#ifdef SHARED_DATA_PATH
	char calib_data[1024];
	sprintf_s(calib_data, "%s/calib_rgbd.xml", SHARED_DATA_PATH);
	glCameraRig->Init(calib_data);
#endif
	// depth image (id:0)
	{
		std::vector<float> camParams4x1 = glCameraRig->GetCameraMatrixCV(0);
		cv::Size imageSize = cv::Size(640, 480);
		kinfu->InitCamera(0, camParams4x1, imageSize);
	}
	// color image (id:1)
	{
		std::vector<float> camParams4x1 = glCameraRig->GetCameraMatrixCV(1);
		cv::Size imageSize = cv::Size(1920, 1080);
		//cv::Size imageSize = cv::Size(640, 360);
		kinfu->InitCamera(1, camParams4x1, imageSize);
	}

	// initialize viewer
	reconsParams = kinfu->GetKinfuParams();
	kinfu->GetVolumeArea(volume_pose, minAB, maxAB);

	resetGlobalView();


	////////////////////////////////////////////////////////////
	// UI: AntTweakBar
	////////////////////////////////////////////////////////////

	// load XML file to import scanned TSDF data
#if 1
	TwAddButton(bar, "LoadXML", [](void* client) { ((Example203*)client)->LoadXML(); }, this, " group='File' key=F1 ");
	TwAddVarRO(bar, "VPM", TwType::TW_TYPE_FLOAT, &reconsParams.voxelsPerMeter, " group='File' ");
	TwAddVarRO(bar, "voxelsX", TwType::TW_TYPE_UINT32, &reconsParams.voxelCountX, " group='File' ");
	TwAddVarRO(bar, "voxelsY", TwType::TW_TYPE_UINT32, &reconsParams.voxelCountY, " group='File' ");
	TwAddVarRO(bar, "voxelsZ", TwType::TW_TYPE_UINT32, &reconsParams.voxelCountZ, " group='File' ");
#endif

	// load images
#if 1
	TwAddButton(bar, "LoadImages", [](void* client) { ((Example203*)client)->LoadImages(); }, this, " group='File' key=F3 ");
	TwAddVarRO(bar, "#Images", TwType::TW_TYPE_INT32, &registered_cameras_count, " group='File' ");
	TwAddVarCB(bar, "Set image", TwType::TW_TYPE_INT32,
		[](const void* value, void* client) { ((Example203*)client)->changeLocalView(*(int*)value); },
		[](void* value, void* client) { *(int*)value = ((Example203*)client)->current_view_num; },
		this, " group='File' ");
#endif

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
	std::vector<TwEnumVal> imageTypeEV = { { ImageType::INVALID, "<EMPTY>" },
		{ ImageType::VIRTUAL_DEPTH, "VirtualDepth" },
		{ ImageType::COLOR, "Color" } };
	TwType twImageType = TwDefineEnum("Raw image", &imageTypeEV[0], (unsigned int)imageTypeEV.size());
	TwAddVarRW(bar, "Raw image", twImageType, &imgType, " group='Local' label='Raw image' ");

	// [TEMPORARY] comment out for a while ...
	/*
	TwAddButton(bar, "Local-init", [](void *client) {
		((Example203*)client)->resetLocalView();
	}, this, "group='Local' label='init' ");
	TwAddVarRW(bar, "Local-rot", TwType::TW_TYPE_QUAT4F, &LocalViewRotation, " group='Local' label='rot' ");
	TwAddVarRW(bar, "Local-posX", TwType::TW_TYPE_FLOAT, &LocalViewPosition.x, " group='Local' label='posX' step=0.01 ");
	TwAddVarRW(bar, "Local-posY", TwType::TW_TYPE_FLOAT, &LocalViewPosition.y, " group='Local' label='posY' step=0.01 ");
	TwAddVarRW(bar, "Local-posZ", TwType::TW_TYPE_FLOAT, &LocalViewPosition.z, " group='Local' label='posZ' step=0.01 ");
	//*/
#endif

	// setting toggles
#if 1
	TwAddVarRW(bar, "Vis-RawImage"  , TwType::TW_TYPE_BOOLCPP, &show_raw_image  , " group='Vis' label='RawImage' ");
	TwAddVarRW(bar, "Vis-Camera"    , TwType::TW_TYPE_BOOLCPP, &show_camera     , " group='Vis' label='Camera' ");
	TwAddVarRW(bar, "Vis-VolumeBox" , TwType::TW_TYPE_BOOLCPP, &show_volume_box , " group='Vis' label='VolumeBox' ");
	TwAddVarRW(bar, "Vis-PointCloud", TwType::TW_TYPE_BOOLCPP, &show_point_cloud, " group='Vis' label='PointCloud' ");
	TwAddVarRW(bar, "Vis-TriMesh"   , TwType::TW_TYPE_BOOLCPP, &show_tri_mesh   , " group='Vis' label='TriMesh' ");
#endif

	return true;
}
void Example203::End()
{
	if (kinfu) delete kinfu;
	if (glCameraRig) delete glCameraRig;
}
bool Example203::Update()
{
	////////////////////////////////////////////////////////////
	// update view matrix
	// global (id:0) / local (id:1) view
	////////////////////////////////////////////////////////////
	glm::mat4 view = glm::translate(glm::mat4(1.0f), -GlobalViewPosition);
	view_matrix[0] = view * glm::toMat4(GlobalViewRotation);
	view_matrix[1] = glCameraRig->GetViewMatrix( (ImageType::COLOR == imgType) ? 1 : 0 );

	// update point cloud data
#if 1
	for (int id = 0; id < 2; id++) {
		kinfu->RenderVolume(id, view_matrix[id], points[id], normals[id], colors[id]);
	}
#endif

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// file I/O
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
void Example203::LoadImages()
{
	std::string folderpath;
	if (cvgl::OpenFolderWindow(folderpath))
	{
		// clear previous data at first
		registered_cameras_pose.clear();
		registered_cameras_img.clear();
		registered_cameras_count = 0;

		// get all color images in the folder
		std::vector<std::string> files = GetColorImagesInFolder(folderpath);
		for (int i = 0; i < files.size(); i++) {
			std::string file = folderpath + "/" + files[i];
			//printf("File #%03d: %s\n", i, file.c_str()); // ***************** [for DEBUG]

			// read BGR image
			cv::Mat img = cv::imread(file.c_str());
			registered_cameras_img.push_back(img);

			// parse color image
			float rx = std::stof(file.substr(file.find("rx") + 2, 9));
			float ry = std::stof(file.substr(file.find("ry") + 2, 9));
			float rz = std::stof(file.substr(file.find("rz") + 2, 9));
			float tx = std::stof(file.substr(file.find("tx") + 2, 7));
			float ty = std::stof(file.substr(file.find("ty") + 2, 7));
			float tz = std::stof(file.substr(file.find("tz") + 2, 7));

			// calculate camera pose from paramters
			glm::mat4 R_view = glm::eulerAngleXYZ(glm::radians(rx), glm::radians(ry), glm::radians(rz));
			glm::mat4 R = glm::inverse(R_view);
			glm::mat4 T = glm::translate(glm::vec3(tx, ty, tz));
			registered_cameras_pose.push_back(T * R);
		}

		// update the number of registered images
		registered_cameras_count = registered_cameras_pose.size();
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
