#ifndef _ARUCO_CALIB
#define _ARUCO_CALIB

#include <aruco/aruco.h>

#include <opencv2/opencv.hpp>


class ArUcoCalib
{
public:
	// constructor
	ArUcoCalib(const cv::Size depthImageSize, const cv::Size colorImageSize)
	{
		imageSize[0] = depthImageSize;
		imageSize[1] = colorImageSize;
	}

	void InitMarker(const char* filePath, float size);
	bool ReadIntrinsicCalib(const int id, const char* filepath);

	void RunSingleCalib(const int id, const char* exportPath = "");
	void RunStereoCalib(const char* exportPath = "");


	////////////////////////////////////////////////////////////
	// simple methods
	////////////////////////////////////////////////////////////
	void DetectMarker(const int id, const cv::Mat image_8u, std::vector<aruco::Marker>& markers)
	{
		markers = MDetector[id].detect(image_8u);
	}
	void Draw(cv::Mat& image_8u, std::vector<aruco::Marker>& markers)
	{
		std::vector<int> indices = MarkerMapConfig.getIndices(markers);
		for (auto idx : indices) markers[idx].draw(image_8u, cv::Scalar(0, 0, 255), 1);
	}

	int Push(const int id,
		const cv::Mat image, const std::vector<aruco::Marker> markers)
	{
		raw_images[id].push_back(image); info_markers[id].push_back(markers);
		return raw_images[id].size();
	}
	void Clear() {
		raw_images[0].clear(); info_markers[0].clear();
		raw_images[1].clear(); info_markers[1].clear();
	}

private:
	aruco::MarkerMap MarkerMapConfig;
	aruco::MarkerDetector MDetector[2];

	std::vector<cv::Mat> raw_images[2];
	std::vector<std::vector<aruco::Marker>> info_markers[2];

	void getMarkerInfo(const int id,
		std::vector<std::vector<cv::Point2f>>& imgPoints,
		std::vector<std::vector<cv::Point3f>>& objPoints,
		std::vector<cv::Mat>& vis_images);
	void getCommonMarkerInfo(
		std::vector<std::vector<cv::Point2f>>& img0Points,
		std::vector<std::vector<cv::Point2f>>& img1Points,
		std::vector<std::vector<cv::Point3f>>& objPoints,
		std::vector<cv::Mat>& vis_images0,
		std::vector<cv::Mat>& vis_images1);

	cv::Size imageSize[2];

	////////////////////////////////////////////////////////////
	// intrinsic parameters
	////////////////////////////////////////////////////////////
	cv::Mat cameraMatrix[2];
	cv::Mat distCoeffs[2];
	bool intrinsic_done[2] = { false, false };
};

#endif
