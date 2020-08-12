#include "Example301.h"
#include <cvgl/PopupWindow.h>


///////////////////////////////////////////////////////////////////////////////
// marker capture for camera calibration
///////////////////////////////////////////////////////////////////////////////
void runCameraCalibration(cvgl::RGBDCamera *camera, int mode)
{
	// stereo calibration
	if (3 == mode) {
		std::string calib_path;

		if (!cvgl::OpenFileWindow(calib_path, "XML (*.xml)\0*.xml\0"s, "Open calibration file for depth camera")) return;
		arucoCalib->ReadIntrinsicCalib(0, calib_path.c_str());

		if (!cvgl::OpenFileWindow(calib_path, "XML (*.xml)\0*.xml\0"s, "Open calibration file for color camera")) return;
		arucoCalib->ReadIntrinsicCalib(1, calib_path.c_str());

		// [TODO] prepare undistortion for depth/color images (?)
	}

	int thresh_infra = 64;
	int thresh_color = 128;

	// main loop
	for (;;) {
		camera->Update();

		// keep original image(s)
		cv::Mat _color = camera->GetColorImage().clone();
		cv::cvtColor(_color, _color, CV_BGR2GRAY);
		cv::cvtColor(_color, _color, CV_GRAY2BGR);

		cv::Mat infra = camera->GetIrImage().clone();
		cv::Mat _infra = cv::Mat(infra.size(), CV_8U);
		cv::convertScaleAbs(infra, _infra, 255.0 / 1200.0);
		cv::cvtColor(_infra, _infra, CV_GRAY2BGR);

		// 
		cv::Mat _color_draw = _color.clone();
		cv::Mat _infra_draw = _infra.clone();

		std::vector<aruco::Marker> markers_infra;
		std::vector<aruco::Marker> markers_color;

		// detect marker from depth image
		if (1 == mode || 3 == mode) {
			cv::threshold(_infra_draw, _infra_draw, thresh_infra, 255, CV_THRESH_BINARY);

			arucoCalib->DetectMarker(0, _infra_draw, markers_infra);
			arucoCalib->Draw(_infra_draw, markers_infra);
			cv::imshow("infrared", _infra_draw);
		}

		// detect marker from color image
		if (2 == mode || 3 == mode) {
			cv::threshold(_color_draw, _color_draw, thresh_color, 255, CV_THRESH_BINARY);

			arucoCalib->DetectMarker(1, _color_draw, markers_color);
			arucoCalib->Draw(_color_draw, markers_color);
			cv::imshow("color", _color_draw);
		}

		// draw images & listen key
		int key = cv::waitKey(1);

		// ESC: escape this program
		if (27 == key) break;

		// Q / E
		if ('q' == key || 'Q' == key) thresh_infra = std::max(thresh_infra - 1, 0);
		if ('e' == key || 'E' == key) thresh_infra = std::min(thresh_infra + 1, 255);

		// A / D
		if ('a' == key || 'A' == key) thresh_color = std::max(thresh_color - 1, 0);
		if ('d' == key || 'D' == key) thresh_color = std::min(thresh_color + 1, 255);

		// SPACE: accumulate image(s)
		if (' ' == key) {
			if (1 == mode && 0 != markers_infra.size()) {
				int count = arucoCalib->Push(0, _infra, markers_infra);
				printf("accumulated image: %d\n", count);
			}
			if (2 == mode && 0 != markers_color.size()) {
				int count = arucoCalib->Push(1, _color, markers_color);
				printf("accumulated image: %d\n", count);
			}
			if (3 == mode && 0 != markers_infra.size() && 0 != markers_color.size()) {
				int count = arucoCalib->Push(0, _infra, markers_infra);
				arucoCalib->Push(1, _color, markers_color);
				printf("accumulated image: %d\n", count);
			}
		}

		// C-key: calibrate accumulated images
		if ('c' == key || 'C' == key) {
			std::string filepath;
			if (cvgl::SaveFileWindow(filepath, "Folder\0*\0\0"s)) {
				if (1 == mode) arucoCalib->RunSingleCalib(0, filepath.c_str());
				if (2 == mode) arucoCalib->RunSingleCalib(1, filepath.c_str());
				if (3 == mode) arucoCalib->RunStereoCalib(filepath.c_str());
			}
		}
	}

	camera->End();
	delete camera;
	return;
}


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
#include <cvgl/RSCamera.h>
#include <cvgl/Kinect2.h>

int main(int argc, char** argv)
{
	_camera = new cvgl::RSCamera();
	//_camera = new cvgl::Kinect2();

	if (!_camera->Init()) {
		fprintf(stderr, "Cannot initialize RGB-D camera.\n");
		return EXIT_FAILURE;
	}

	// initialize ArUco marker for camera calibration
	char marker_path[1024];
	sprintf_s(marker_path, "%s/aruco_calibration_board_a4.yml", SHARED_DATA_PATH);

	cv::Size depth_size = _camera->GetDepthImageSize();
	cv::Size color_size = _camera->GetColorImageSize();

	arucoCalib = new ArUcoCalib(depth_size, color_size);
	arucoCalib->InitMarker(marker_path, 0.034f);

	////////////////////////////////////////////////////////////
	// select the mode
	////////////////////////////////////////////////////////////
	int mode;
	printf("1: Run single camera (depth) calibration\n");
	printf("2: Run single camera (color) calibration\n");
	printf("3: Run stereo camera calibration\n");
	printf("Others: Run checking program\n");
	printf("\tSelect program mode: ");
	scanf("%d", &mode);

	if (1 == mode || 2 == mode || 3 == mode)
	{
		runCameraCalibration(_camera, mode);
		return EXIT_SUCCESS;
	}

	// [TODO]

	return EXIT_SUCCESS;
}
