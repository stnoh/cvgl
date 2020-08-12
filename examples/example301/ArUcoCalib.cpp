#include "ArUcoCalib.h"
#include <direct.h> // to use "_mkdir"


///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////
void ArUcoCalib::InitMarker(const char* filePath, float size)
{
	MarkerMapConfig.readFromFile(filePath);
	MarkerMapConfig = MarkerMapConfig.convertToMeters(size);

	for (int id = 0; id < 2; id++)
	{
		aruco::MarkerDetector::Params params;
		/*
		params._borderDistThres = .025;
		params._thresParam1 = 5;
		params._thresParam1_range = 5;
		params._cornerMethod = aruco::MarkerDetector::SUBPIX;
		params._subpix_wsize = (15. / 2000.) * imageSize[id].width;
		//*/
//		MDetector[id].setParameters(params);
		MDetector[id].setDictionary(MarkerMapConfig.getDictionary());
	}
}
bool ArUcoCalib::ReadIntrinsicCalib(const int id, const char* filepath)
{
	cv::FileStorage fs(filepath, cv::FileStorage::READ);

	if (!fs.isOpened()) return false;

	cv::Mat cameraMatrix_xml;
	cv::Mat distCoeffs_xml;
	cv::Size imageSize_xml;

	fs["image_size"] >> imageSize_xml;
	fs["camera_matrix"] >> cameraMatrix_xml;
	fs["dist_coeffs"] >> distCoeffs_xml;

	// scale ...
	float scale_x = (float)imageSize[id].width / (float)imageSize_xml.width;
	float scale_y = (float)imageSize[id].height / (float)imageSize_xml.height;

	cameraMatrix[id] = cameraMatrix_xml;
	cameraMatrix[id].at<float>(cv::Point(0, 0)) *= scale_x;
	cameraMatrix[id].at<float>(cv::Point(1, 1)) *= scale_y;
	cameraMatrix[id].at<float>(cv::Point(2, 0)) *= scale_x;
	cameraMatrix[id].at<float>(cv::Point(2, 1)) *= scale_y;

	distCoeffs[id] = distCoeffs_xml;

	printf("\nCamera #%d:\n", id);
	std::cout << "camera matrix: " << cameraMatrix[id] << std::endl;
	std::cout << "distortion coefficients: " << distCoeffs[id] << std::endl;

	intrinsic_done[id] = true;
	return true;
}


///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////
void ArUcoCalib::RunSingleCalib(const int id, const char* exportPath)
{
	// get data from container
	std::vector<std::vector<cv::Point2f>> imgPoints;
	std::vector<std::vector<cv::Point3f>> objPoints;
	std::vector<cv::Mat> visImages;
	getMarkerInfo(id, imgPoints, objPoints, visImages);

	// calibration informations
	cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_32F);
	cv::Mat distCoeffs(5, 1, CV_32F); // 5-th order: {k1, k2, p1, p2, k3}
	std::vector<cv::Mat> rvecs;
	std::vector<cv::Mat> tvecs;

	int flags = 0;
	double error = cv::calibrateCamera(objPoints, imgPoints, imageSize[id], cameraMatrix, distCoeffs, rvecs, tvecs, flags);

	// show information
	std::cout << "image_size = " << imageSize << std::endl;
	std::cout << "camera_matrix = " << cameraMatrix << std::endl;
	std::cout << "dist_coeffs = " << distCoeffs << std::endl;
	std::cout << "calibration_error = " << error << std::endl;

	// export files
	if (0 > std::strcmp("", exportPath)) {
		_mkdir(exportPath);

		// export images
		std::vector<int> writeParams = { CV_IMWRITE_PNG_COMPRESSION, 0 };
		for (int i = 0; i < raw_images[id].size(); i++) {
			char buf[1024];
			snprintf(buf, 1024, "%s/image_raw_%03d.png", exportPath, i);
			cv::imwrite(buf, raw_images[id][i], writeParams);

			snprintf(buf, 1024, "%s/image_vis_%03d.png", exportPath, i);
			cv::imwrite(buf, visImages[i], writeParams);
		}

		// export calibration data in xml format
		char path[1024];
		snprintf(path, 1024, "%s/calib.xml", exportPath);
		cv::FileStorage fs(path, cv::FileStorage::WRITE);

		fs << "image_size" << imageSize[id];
		fs << "camera_matrix" << cameraMatrix;
		fs << "dist_coeffs" << distCoeffs;
		fs << "calibration_error" << error;
		fs << "rvecs" << rvecs;
		fs << "tvecs" << tvecs;
	}
}
void ArUcoCalib::RunStereoCalib(const char* exportPath)
{
	// check if intrinsic calibration is not done, then do it here.
	if (!intrinsic_done[0])
	{
		printf("Please calibrate intrinsic parameters of camera0.\n");
		return;
	}
	if (!intrinsic_done[1])
	{
		printf("Please calibrate intrinsic parameters of camera1.\n");
		return;
	}

	// get data from container
	std::vector<std::vector<cv::Point2f>> imgPoints[2];
	std::vector<std::vector<cv::Point3f>> objPoints;
	std::vector<cv::Mat> visImages[2];
	getCommonMarkerInfo(imgPoints[0], imgPoints[1], objPoints, visImages[0], visImages[1]);

	// calibration informations
	cv::TermCriteria criteria = cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 100, 1e-5);
	cv::Mat Rmat, T, E, F;
	int flags = CV_CALIB_USE_INTRINSIC_GUESS | CV_CALIB_FIX_INTRINSIC;
	double error = cv::stereoCalibrate(objPoints, imgPoints[0], imgPoints[1],
		cameraMatrix[0], distCoeffs[0], cameraMatrix[1], distCoeffs[1],
		imageSize[0], Rmat, T, E, F, criteria, flags);

	cv::Vec3f Tvec(T.at<double>(0), T.at<double>(1), T.at<double>(2));

	// show information
	std::cout << "stereo calibration error = " << error << std::endl;
	std::cout << "Rotation matrix = " << Rmat << std::endl;
	std::cout << "Translation vector = " << Tvec << std::endl;

	// export files
	if (0 > std::strcmp("", exportPath)) {
		_mkdir(exportPath);

		// export images
		std::vector<int> writeParams = { CV_IMWRITE_PNG_COMPRESSION, 0 };
		for (int i = 0; i < raw_images[0].size(); i++) {
			char buf[1024];

			for (int id = 0; id < 2; id++) {
				snprintf(buf, 1024, "%s/image%d_raw_%03d.png", exportPath, id, i);
				cv::imwrite(buf, raw_images[id][i], writeParams);

				snprintf(buf, 1024, "%s/image%d_vis_%03d.png", exportPath, id, i);
				cv::imwrite(buf, visImages[id][i], writeParams);
			}
		}

		// export calibration data in xml format
		char path[1024];
		snprintf(path, 1024, "%s/calib_rgbd.xml", exportPath);
		cv::FileStorage fs(path, cv::FileStorage::WRITE);

		// this lines need to modify in manual way ...
		cvWriteComment(*fs, "from manual input", 0);
		fs << "depth_camera_focal" << 480.0;
		fs << "color_camera_focal" << 700.0; // good focal length for QHD (960x540)
		fs << "depth_clipping_near" << 200;
		fs << "depth_clipping_far" << 1200;

		// depth camera intrinsics
		cvWriteComment(*fs, "from intrinsic calibration for depth", 0);
		fs << "depth_image_size" << imageSize[0];
		fs << "depth_camera_matrix" << cameraMatrix[0];
		fs << "depth_dist_coeffs" << distCoeffs[0];

		// color camera intrinsics
		cvWriteComment(*fs, "from intrinsic calibration for color", 0);
		fs << "color_image_size" << imageSize[1];
		fs << "color_camera_matrix" << cameraMatrix[1];
		fs << "color_dist_coeffs" << distCoeffs[1];

		// depth-color camera extrinsics
		cvWriteComment(*fs, "from extrinsic calibration", 0);
		fs << "depth_to_color_rmat" << Rmat;
		fs << "depth_to_color_tvec" << Tvec;
		fs << "calibration_error" << error;
	}
}


///////////////////////////////////////////////////////////////////////////////
// private methods
///////////////////////////////////////////////////////////////////////////////
void ArUcoCalib::getMarkerInfo(
	const int id,
	std::vector<std::vector<cv::Point2f>>& imgPoints,
	std::vector<std::vector<cv::Point3f>>& objPoints,
	std::vector<cv::Mat>& visImages)
{
	imgPoints.clear();
	objPoints.clear();
	visImages.clear();

	auto getMarker2d_3d = [&](std::vector<cv::Point2f> &p2d, std::vector<cv::Point3f> &p3d, const std::vector<aruco::Marker> markers) {
		p2d.clear();
		p3d.clear();

		for (int i = 0; i < markers.size(); i++) {
			for (auto j = 0; j < MarkerMapConfig.size(); j++)
				if (MarkerMapConfig[j].id == markers[i].id) {
					for (int k = 0; k < 4; k++) {
						p2d.push_back(markers[i][k]);
						p3d.push_back(MarkerMapConfig[j][k]);
					}
				}
		}
	};

	for (int i = 0; i < raw_images[id].size(); i++)
	{
		std::vector<aruco::Marker> markers = info_markers[id][i];

		// get 2D/3D points from marker info.
		std::vector<cv::Point2f> p2d;
		std::vector<cv::Point3f> p3d;
		getMarker2d_3d(p2d, p3d, markers);
		imgPoints.push_back(p2d);
		objPoints.push_back(p3d);

		// convert to color if image is grayscale
		cv::Mat vis_image = raw_images[id][i].clone();
		if (1 == vis_image.channels()) cv::cvtColor(vis_image, vis_image, CV_GRAY2BGR);

		// draw marker info. on image
		Draw(vis_image, markers);
		visImages.push_back(vis_image);
	}
}
void ArUcoCalib::getCommonMarkerInfo(
	std::vector<std::vector<cv::Point2f>>& img0Points,
	std::vector<std::vector<cv::Point2f>>& img1Points,
	std::vector<std::vector<cv::Point3f>>& objPoints,
	std::vector<cv::Mat>& visImages0,
	std::vector<cv::Mat>& visImages1)
{
	img0Points.clear();
	img1Points.clear();
	objPoints.clear();
	visImages0.clear();
	visImages1.clear();

	auto getCommonMarker2d_3d = [&](
		std::vector<cv::Point2f> &p2d0, std::vector<cv::Point2f> &p2d1, std::vector<cv::Point3f> &p3d,
		const std::vector<aruco::Marker> markers0, const std::vector<aruco::Marker> markers1 ) {

		p2d0.clear();
		p2d1.clear();
		p3d.clear();

		// find "common" detected markers
		for (int k = 0; k < MarkerMapConfig.size(); k++) {
			int id = MarkerMapConfig[k].id;

			// from image0
			int idx0 = -1;
			for (int i = 0; i < markers0.size(); i++) {
				if (id == markers0[i].id) {
					idx0 = i;
					continue;
				}
			}
			if (-1 == idx0) continue;

			// from image1
			int idx1 = -1;
			for (int j = 0; j < markers1.size(); j++) {
				if (id == markers1[j].id) {
					idx1 = j;
					continue;
				}
			}
			if (-1 == idx1) continue;

			for (int p = 0; p < 4; p++) {
				cv::Vec3f obj_p3d = MarkerMapConfig[k][p];
				cv::Vec2f img_p2d0 = markers0[idx0][p];
				cv::Vec2f img_p2d1 = markers1[idx1][p];

				p3d.push_back(obj_p3d);
				p2d0.push_back(img_p2d0);
				p2d1.push_back(img_p2d1);
			}
		}
	};

	for (int n = 0; n < raw_images[0].size(); n++)
	{
		std::vector<aruco::Marker> markers[2];
		for (int id = 0; id < 2; id++) {
			markers[id] = info_markers[id][n];
		}

		// get 2D/3D points from marker info.
		std::vector<cv::Point2f> p2d[2];
		std::vector<cv::Point3f> p3d;
		getCommonMarker2d_3d(p2d[0], p2d[1], p3d, markers[0], markers[1]);
		img0Points.push_back(p2d[0]);
		img1Points.push_back(p2d[1]);
		objPoints.push_back(p3d);

		// convert to color if image is grayscale and draw common points for calibration
		cv::Mat vis_image[2];
		for (int id = 0; id < 2; id++) {
			vis_image[id] = raw_images[id][n].clone();
			if (1 == vis_image[id].channels()) cv::cvtColor(vis_image[id], vis_image[id], CV_GRAY2BGR);

			// draw "common" marker points
			int radius = vis_image[id].rows / 240;
			for (int i = 0; i < p2d[id].size(); i++) {
				cv::circle(vis_image[id], p2d[id][i], radius, cv::Scalar(0, 0, 255), radius);
			}
		}

		// draw marker info. on image
		visImages0.push_back(vis_image[0]);
		visImages1.push_back(vis_image[1]);
	}
}
