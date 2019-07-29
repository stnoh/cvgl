/******************************************************************************
C++ facade class for RealSense camera API with OpenCV containers
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#ifndef RSSDK_FACADE
#define RSSDK_FACADE

#include "cvgl/RGBDCamera.h"

#include <pxcsensemanager.h>

#include <opencv2/opencv.hpp>

namespace cvgl {

class RSCamera : public RGBDCamera
{
public:
	RSCamera();
	virtual ~RSCamera();

	bool Init();
	void End();

	bool InitLive() { return Init(); }

	bool Update();

private:
	// camera-dependent APIs
	bool updateRawImages();

	bool initRawStreams(const bool sync = false);
	bool initRSCameraSetting();
	
	void releaseRSManagers();

	PXCSession        *session = nullptr;
	PXCSenseManager   *psm     = nullptr;
	PXCCaptureManager *pcm_ref = nullptr;
};

}

#endif
