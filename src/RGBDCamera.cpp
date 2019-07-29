/******************************************************************************
Abstract class for RGB-D Camera with full calibration control by OpenCV
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include "cvgl/RGBDCamera.h"

using namespace cvgl;

///////////////////////////////////////////////////////////////////////////////
// ctor / dtor
///////////////////////////////////////////////////////////////////////////////
RGBDCamera::RGBDCamera()
{
	// initialize OpenCV image containers
	ResizeImageContainers();
}
RGBDCamera::~RGBDCamera()
{
	// finalize OpenCV image containers
	color_image.release();
	depth_image.release();
	ir_image.release();

	colored_layer.release();

	// finalize point cloud
	points.clear();
	colors.clear();
}


///////////////////////////////////////////////////////////////////////////////
// common update routine
///////////////////////////////////////////////////////////////////////////////
bool RGBDCamera::Update()
{
	if (_calib3d_data)
	{
		RectifyImages();
		UpdatePointCloud();
	}

	return true;
}
void RGBDCamera::ResizeImageContainers()
{
	color_image = cv::Mat(color_size, CV_8UC3);
	depth_image = cv::Mat(depth_size, CV_16UC1);
	ir_image    = cv::Mat(depth_size, CV_16UC1);

	colored_layer = cv::Mat(depth_size, CV_8UC4);

	int numOfV = depth_size.area();
	points = std::vector<glm::vec3>  (numOfV);
	colors = std::vector<glm::u8vec3>(numOfV);
}


///////////////////////////////////////////////////////////////////////////////
// calibration
///////////////////////////////////////////////////////////////////////////////
bool RGBDCamera::ReadCalibrationData()
{
	cv::FileStorage fs(calibXML, cv::FileStorage::READ);

	if (!fs.isOpened()) {
		fprintf(stderr, "ERROR: cannot open file: %s\n", calibXML.c_str());
		return false;
	}

	////////////////////////////////////////////////////////////
	// read XML file to get calibration data ...
	////////////////////////////////////////////////////////////
	cv::Mat  color_cameraMatrix_raw, color_distCoeffs;
	cv::Size color_imageSize;
	float    color_focalLength;

	cv::Mat  depth_cameraMatrix_raw, depth_distCoeffs;
	cv::Size depth_imageSize;
	float    depth_focalLength;

	cv::Mat   rMat;
	cv::Vec3f tVec;

	try {
		fs["color_camera_matrix"] >> color_cameraMatrix_raw;
		fs["color_dist_coeffs"]   >> color_distCoeffs;
		fs["color_image_size"]    >> color_imageSize;
		fs["color_camera_focal"]  >> color_focalLength;

		fs["depth_camera_matrix"] >> depth_cameraMatrix_raw;
		fs["depth_dist_coeffs"]   >> depth_distCoeffs;
		fs["depth_image_size"]    >> depth_imageSize;
		fs["depth_camera_focal"]  >> depth_focalLength;

		fs["depth_clipping_near"] >> depth_clipping_near;
		fs["depth_clipping_far"]  >> depth_clipping_far;

		fs["depth_to_color_rmat"] >> rMat;
		fs["depth_to_color_tvec"] >> tVec;
	}
	catch (const cv::Exception e)
	{
		fprintf(stderr, "ERROR: read XML.\n");
		return false;
	}

	////////////////////////////////////////////////////////////
	// rescaling intrinsic camera calibration parameters
	////////////////////////////////////////////////////////////
	auto GetIntrinsicCalibRemap = [](cv::Size imageSize,
		cv::Mat cameraMatrix_raw, cv::Mat distCoeffs, cv::Size calibImageSize, float focalLength,
		cv::Mat& cameraMatrix_new, cv::Mat& remap_x, cv::Mat& remap_y)->void
	{
		float scale_x = (float)imageSize.width  / (float)calibImageSize.width;
		float scale_y = (float)imageSize.height / (float)calibImageSize.height;

		// camera matrix from calibration data
		cv::Mat cameraMatrix_scaled = cv::Mat::eye(3, 3, CV_32F);
		cameraMatrix_scaled.at<float>(cv::Point(0, 0)) = scale_x * cameraMatrix_raw.at<double>(cv::Point(0, 0));
		cameraMatrix_scaled.at<float>(cv::Point(1, 1)) = scale_y * cameraMatrix_raw.at<double>(cv::Point(1, 1));
		cameraMatrix_scaled.at<float>(cv::Point(2, 0)) = scale_x * cameraMatrix_raw.at<double>(cv::Point(2, 0));
		cameraMatrix_scaled.at<float>(cv::Point(2, 1)) = scale_y * cameraMatrix_raw.at<double>(cv::Point(2, 1));

		// (output 1) new camera matrix
		cameraMatrix_new = cv::Mat::eye(3, 3, CV_32F);
		cameraMatrix_new.at<float>(cv::Point(0, 0)) = focalLength * scale_x;
		cameraMatrix_new.at<float>(cv::Point(1, 1)) = focalLength * scale_y;
		cameraMatrix_new.at<float>(cv::Point(2, 0)) = 0.5f * imageSize.width;
		cameraMatrix_new.at<float>(cv::Point(2, 1)) = 0.5f * imageSize.height;

		// (output 2) get remap data for undistortion
		cv::initUndistortRectifyMap(cameraMatrix_scaled, distCoeffs,
			cv::Mat::eye(3, 3, CV_32F), cameraMatrix_new, imageSize, CV_32FC1,
			remap_x, remap_y);
	};

	// intrinsic calibration on color camera
	{
		cv::Mat cameraMatrix_c;
		GetIntrinsicCalibRemap(color_size,
			color_cameraMatrix_raw, color_distCoeffs, color_imageSize, color_focalLength,
			cameraMatrix_c, remap_color_x, remap_color_y);

		fx_c = cameraMatrix_c.at<float>(cv::Point(0, 0));
		fy_c = cameraMatrix_c.at<float>(cv::Point(1, 1));
		cx_c = cameraMatrix_c.at<float>(cv::Point(2, 0));
		cy_c = cameraMatrix_c.at<float>(cv::Point(2, 1));
	}

	// intrinsic calibration on depth camera
	{
		cv::Mat cameraMatrix_d;
		GetIntrinsicCalibRemap(depth_size,
			depth_cameraMatrix_raw, depth_distCoeffs, depth_imageSize, depth_focalLength,
			cameraMatrix_d, remap_depth_x, remap_depth_y);

		fx_d = cameraMatrix_d.at<float>(cv::Point(0, 0));
		fy_d = cameraMatrix_d.at<float>(cv::Point(1, 1));
		cx_d = cameraMatrix_d.at<float>(cv::Point(2, 0));
		cy_d = cameraMatrix_d.at<float>(cv::Point(2, 1));
	}

	////////////////////////////////////////////////////////////
	// extrinsic: depth to color
	////////////////////////////////////////////////////////////
	Depth2ColorR = rMat;
	Depth2ColorT = tVec; // it is already in [m]

	_calib3d_data = true;

	return true;
}
void RGBDCamera::GetCalibData(const int id, std::vector<float>& camParams4x1, glm::mat4& pose)
{
	glm::mat4 view_GL = glm::mat4(1.0f);
	float fx, fy, cx, cy;

	// color camera id = 1
	if (1 == id) {
		fx = fx_c / (float)color_size.width;
		fy = fy_c / (float)color_size.height;
		cx = cx_c / (float)color_size.width;
		cy = cy_c / (float)color_size.height;

		for (int j = 0; j < 3; j++) 
		for (int i = 0; i < 3; i++) {
			view_GL[j][i] = Depth2ColorR(i, j);
		}

		// [CAUTION] transform from CV(x,y,z) to GL(x,-y,-z)
		view_GL[0][1] = -view_GL[0][1];
		view_GL[0][2] = -view_GL[0][2];
		view_GL[1][0] = -view_GL[1][0];
		view_GL[2][0] = -view_GL[2][0];

		view_GL[3][0] = +Depth2ColorT.x;
		view_GL[3][1] = -Depth2ColorT.y;
		view_GL[3][2] = -Depth2ColorT.z;
	}
	else {
		fx = fx_d / (float)depth_size.width;
		fy = fy_d / (float)depth_size.height;
		cx = cx_d / (float)depth_size.width;
		cy = cy_d / (float)depth_size.height;
	}

	camParams4x1 = { fx, fy, cx, cy };
	pose = glm::inverse(view_GL);
}


///////////////////////////////////////////////////////////////////////////////
// internal methods
///////////////////////////////////////////////////////////////////////////////
void RGBDCamera::RectifyImages()
{
	const int method = CV_INTER_LINEAR;

	// copy
	cv::Mat color_raw = color_image.clone();
	cv::Mat depth_raw = depth_image.clone();
	cv::Mat ir_raw = ir_image.clone();

	// undistort images
	cv::remap(color_raw, color_image, remap_color_x, remap_color_y, method);
	cv::remap(depth_raw, depth_image, remap_depth_x, remap_depth_y, CV_INTER_NN); // [CAUTION] DO NOT INTERPOLATE depth image.
	cv::remap(ir_raw   , ir_image   , remap_depth_x, remap_depth_y, method);
}
void RGBDCamera::UpdatePointCloud()
{
	// depth image to point cloud
	int d_w = depth_size.width;
	int d_h = depth_size.height;
	int c_w = color_size.width;
	int c_h = color_size.height;

	int numOfV = depth_size.area();

	// clear color as black
	memset(colored_layer.data, 0, sizeof(unsigned char) * 4 * numOfV);

	unsigned short* depth_data = (unsigned short*)depth_image.data;

	for (int j = 0; j < d_h; j++)
	for (int i = 0; i < d_w; i++) {
		int vidx = i + j*d_w;

		unsigned short depth = depth_data[vidx];
		float z = mm2m * depth;

		if (depth < depth_clipping_near || depth > depth_clipping_far) z = 0.0f;

		float x = z * (i - cx_d) / fx_d;
		float y = z * (j - cy_d) / fy_d;

		cv::Point3f p(x, y, z);
		cv::Point3f pc = Depth2ColorR * p + Depth2ColorT;

		int ci = (int)(fx_c * pc.x / pc.z + cx_c);
		int cj = (int)(fy_c * pc.y / pc.z + cy_c);

		glm::u8vec3 c = glm::u8vec3(128, 128, 128); // fallback as gray
		if (0 <= ci && ci <= c_w - 1 &&
			0 <= cj && cj <= c_h - 1) {
			int pidx = ci + cj * c_w;

			c.b = color_image.data[3 * pidx + 0];
			c.g = color_image.data[3 * pidx + 1];
			c.r = color_image.data[3 * pidx + 2];
			const unsigned char a = 0;

			// write BGRA color at once
			int bgra = (a << 24 | c.r << 16 | c.g << 8 | c.b);
			((int*)colored_layer.data)[vidx] = bgra;
		}
		else {
			// delete non-colored depth pixel for KinectFusion
			((unsigned short*)depth_image.data)[vidx] = 0;
		}

		points[vidx] = glm::vec3(x, -y, -z); // [CAUTION] CV to GL
		colors[vidx] = c;
	}
}
