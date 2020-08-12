#include <cvgl/RGBDViewer.h>
#include <cvgl/Kinect2.h>


///////////////////////////////////////////////////////////////////////////////
// simple class for Application
///////////////////////////////////////////////////////////////////////////////
class Example202 : public RGBDViewer
{
public:
	Example202(const int width, const int height) : RGBDViewer(width, height) {};

	bool Init()
	{
		// initialize camera object
		camera = new cvgl::Kinect2();

		// read calibration data (if exists)
#ifdef SHARED_DATA_PATH
		char calib_data[1024];
		sprintf_s(calib_data, "%s/calib_kinect2.xml", SHARED_DATA_PATH);
		camera->ReadCalibrationData(calib_data);
#endif

		// instantiate color/depth camera rig
		glCameraRig = new cvgl::GLCameraRig();
		glCameraRig->Init(camera);

		bool init_cam = ((cvgl::Kinect2*)camera)->Init();
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
	Example202 app(1024, 424); // depth: 512x424

	app.SetInternalProcess(true); // always update for RGBD camera
	app.run();

	app.End();
	return EXIT_SUCCESS;
}
