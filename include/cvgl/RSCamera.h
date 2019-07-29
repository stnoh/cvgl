/******************************************************************************
C++ facade class for RealSense camera API with OpenCV containers
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#ifndef RSSDK_FACADE
#define RSSDK_FACADE

#include <pxcsensemanager.h>

#include <opencv2/opencv.hpp>

namespace cvgl{

class RSCamera
{
public:
	RSCamera();
	virtual ~RSCamera();

	bool InitLive();
	void End();

	bool Update();

	const cv::Mat& GetColorImage() { return color_image; }
	const cv::Mat& GetDepthImage() { return depth_image; }
	const cv::Mat& GetIrImage()    { return ir_image; }

	const cv::Size& GetColorImageSize() { return color_size; }
	const cv::Size& GetDepthImageSize() { return depth_size; }

protected:
	// camera-independent but sealed APIs
	void ResizeImageContainers();

	cv::Mat color_image;
	cv::Mat depth_image;
	cv::Mat ir_image;

	cv::Size color_size = cv::Size(1, 1);
	cv::Size depth_size = cv::Size(1, 1);

private:
	// camera-dependent APIs
	bool updateRawImages();
	bool initRawStreams(const bool sync = false);

	void releaseRSManagers();

	PXCSession        *session = nullptr;
	PXCSenseManager   *psm     = nullptr;
	PXCCaptureManager *pcm_ref = nullptr;
};

}

#endif
