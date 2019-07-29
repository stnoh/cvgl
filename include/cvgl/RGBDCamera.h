/******************************************************************************
Abstract class for RGB-D Camera with full calibration control by OpenCV
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#ifndef RGBD_CAMERA
#define RGBD_CAMERA

#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>

namespace cvgl {

const float mm2m = 0.001f; // [mm] to [m]
const int record_frame_max = (int)(1.0 * 60 * 30); // 1.0[min] x 60[sec/min] x 30[frame/sec]

class RGBDCamera
{
public:
	RGBDCamera();
	virtual ~RGBDCamera();

	////////////////////////////////////////
	// main routine
	////////////////////////////////////////
	virtual bool Init() = 0;
	virtual void End()  = 0;

	virtual bool Update();
	virtual void ResizeImageContainers();


	////////////////////////////////////////
	// related to 3D calibration
	////////////////////////////////////////
	bool ReadCalibrationData();
	bool ReadCalibrationData(const std::string& xmlFile) {
		calibXML = std::string(xmlFile);
		return ReadCalibrationData();
	}
	void GetCalibData(const int id, std::vector<float>& camParams4x1, glm::mat4& pose);

	std::string calibXML = "./calib_rgbd.xml";


	////////////////////////////////////////
	// getter for image containers
	////////////////////////////////////////
	const cv::Mat& GetColorImage() { return color_image; }
	const cv::Mat& GetDepthImage() { return depth_image; }
	const cv::Mat& GetIrImage()    { return ir_image; }

	const cv::Mat& GetColoredLayer() { return colored_layer; }

	void GetPointCloud(std::vector<glm::vec3>& points)
	{
		points = this->points;
	}
	void GetPointCloud(std::vector<glm::vec3>& points, std::vector<glm::u8vec3>& colors)
	{
		points = this->points;
		colors = this->colors;
	}

	const cv::Size& GetColorImageSize() { return color_size; };
	const cv::Size& GetDepthImageSize() { return depth_size; };

protected:

	////////////////////////////////////////////////////////////
	// image containers
	////////////////////////////////////////////////////////////
	cv::Mat color_image;
	cv::Mat depth_image;
	cv::Mat ir_image;

	cv::Mat colored_layer; // color on depth image

	std::vector<glm::vec3>   points;
	std::vector<glm::u8vec3> colors;

	cv::Size color_size = cv::Size(1, 1);
	cv::Size depth_size = cv::Size(1, 1);


	////////////////////////////////////////////////////////////
	// calibration (undistortion, color-depth mapping)
	////////////////////////////////////////////////////////////
	void RectifyImages();
	void UpdatePointCloud();

	cv::Mat remap_depth_x, remap_depth_y;
	cv::Mat remap_color_x, remap_color_y;

	float fx_d, fy_d, cx_d, cy_d;
	float fx_c, fy_c, cx_c, cy_c;

	unsigned short depth_clipping_near;
	unsigned short depth_clipping_far;

	cv::Matx33f Depth2ColorR;
	cv::Point3f Depth2ColorT; // in [m]

	bool _calib3d_data = false;
};

}

#endif
