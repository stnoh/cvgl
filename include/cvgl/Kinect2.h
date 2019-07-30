/******************************************************************************
C++ facade class for Kinect v2 API with OpenCV & glm containers
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#ifndef RGBD_KINECT2
#define RGBD_KINECT2

#include "cvgl/RGBDCamera.h"

#include <Kinect.h>
#include <opencv2/opencv.hpp>

#include <vector>
#include <glm/glm.hpp>

namespace cvgl {

class Kinect2 : public RGBDCamera
{
public:
	Kinect2();
	virtual ~Kinect2();

	bool Init();
	void End();

	// overrided functions
	bool Update();
	void ResizeImageContainers();

private:

	// camera-dependent APIs
	bool initRawStream();

	bool initColorImage();
	bool initDepthImage();
	bool initIrImage();

	bool initKinect2Setting();

	bool updateColorImage();
	bool updateDepthImage();
	bool updateIrImage();

	bool update3DInfo();

	IKinectSensor *pSensor = nullptr;

	IColorFrameReader* pColorFrameReader = nullptr;
	IDepthFrameReader* pDepthFrameReader = nullptr;
	IInfraredFrameReader* pIrFrameReader = nullptr;

	ICoordinateMapper* pCoordinateMapper = nullptr;
};

}

#endif
