#include <cvgl/RGBDViewer.h>
#include <cvgl/RSCamera.h>


///////////////////////////////////////////////////////////////////////////////
// simple class for Application
///////////////////////////////////////////////////////////////////////////////
class Example201 : public RGBDViewer
{
public:
	Example201(const int width, const int height) : RGBDViewer(width, height) {};

	bool Init()
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

		bool init_cam = ((cvgl::RSCamera*)camera)->InitLive();
		bool init_gui = (RGBDViewer::Init());

		return init_gui && init_cam;
	}

	bool Update()
	{
		bool updated = (RGBDViewer::Update());

		return updated;
	}
};


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	Example201 app(1280, 480); // depth: 640x480

	app.SetInternalProcess(true); // always update for RGBD camera
	app.run();

	app.End();
	return EXIT_SUCCESS;
}
