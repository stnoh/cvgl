/******************************************************************************
C++ facade class for RealSense camera API with OpenCV containers
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include "cvgl/RSCamera.h"
#include <RealSense/Session.h>

#define CHECK_STATUS(sts, msg) if (sts < PXC_STATUS_NO_ERROR) { fprintf(stderr, "ERROR: %s.\n", msg); return false; }
#define CHECK_EXIST(ptr, msg) if (!ptr) { fprintf(stderr, "ERROR: %s.\n", msg); End(); return false; }

using namespace cvgl;


///////////////////////////////////////////////////////////////////////////////
// ctor / dtor
///////////////////////////////////////////////////////////////////////////////
RSCamera::RSCamera()
{
	pcm_ref = nullptr;
	psm     = nullptr;

	session = PXCSession::CreateInstance();
	if (!session) {
		fprintf(stderr, "ERROR: cannot create session in RealSense SDK.\n");
		exit(1);
	}

	// initialize OpenCV image containers
	ResizeImageContainers();
}
RSCamera::~RSCamera()
{
	End(); // for safety

	if (session) session->Release();

	// finalize OpenCV image containers
	color_image.release();
	depth_image.release();
	ir_image.release();
}


///////////////////////////////////////////////////////////////////////////////
// initialize/finalize
///////////////////////////////////////////////////////////////////////////////
bool RSCamera::InitLive()
{
	releaseRSManagers();

	// create PXCSenseManager
	psm = session->CreateSenseManager();
	if (!psm) {
		fprintf(stderr, "ERROR: Unable to create the PXCSenseManager.\n");
		End();
		return false;
	}

	initRawStreams(true);

	pcm_ref = psm->QueryCaptureManager();
	CHECK_EXIST(pcm_ref, "Unable to query the PXCCaptureManager");

	pxcStatus sts = psm->Init();
	if (sts < PXC_STATUS_NO_ERROR) {
		fprintf(stderr, "ERROR: Unable to create the PXCCaptureManager.\n");
		End();
		return false;
	}

	ResizeImageContainers();

	return true;
}
void RSCamera::End()
{
	releaseRSManagers();
}


///////////////////////////////////////////////////////////////////////////////
// main routine
///////////////////////////////////////////////////////////////////////////////
bool RSCamera::Update()
{
	if (!updateRawImages())
	{
		fprintf(stderr, "ERROR: cannot acquire the latest frame.\n");
		return false;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// camera-independent but sealed APIs
///////////////////////////////////////////////////////////////////////////////
void RSCamera::ResizeImageContainers()
{
	color_image = cv::Mat(color_size, CV_8UC3);
	depth_image = cv::Mat(depth_size, CV_16UC1);
	ir_image    = cv::Mat(depth_size, CV_16UC1);
}


///////////////////////////////////////////////////////////////////////////////
// camera-dependent APIs
///////////////////////////////////////////////////////////////////////////////
bool RSCamera::updateRawImages()
{
	if (!psm) return false;

	// try to get latest frame
	pxcStatus sts = psm->AcquireFrame(false);
	CHECK_STATUS(sts, "Unable to acquire current frame from camera");

	PXCCapture::Sample *sample = psm->QuerySample();
	PXCImage *colorIm = sample->color;
	PXCImage *depthIm = sample->depth;
	PXCImage *infraIm = sample->ir;

	auto CopyImage = [](PXCImage* src, cv::Mat& dst, PXCImage::PixelFormat format, int size_t) {
		if (src) {
			PXCImage::ImageData data_image;
			src->AcquireAccess(PXCImage::ACCESS_READ, format, &data_image);
			memcpy(dst.data, data_image.planes[0], size_t);
			src->ReleaseAccess(&data_image);
		}
	};
	CopyImage(colorIm, color_image, PXCImage::PIXEL_FORMAT_RGB24, color_size.area() * sizeof(uchar) * 3);
	CopyImage(depthIm, depth_image, PXCImage::PIXEL_FORMAT_DEPTH, depth_size.area() * sizeof(ushort));
	CopyImage(infraIm, ir_image   , PXCImage::PIXEL_FORMAT_Y16  , depth_size.area() * sizeof(ushort));

	// release this frame
	psm->ReleaseFrame();

	return true;
}

bool RSCamera::initRawStreams(const bool sync)
{
	pxcStatus sts;

	// set maximum raw image size with 60fps
	int color_fps = 60;
	color_size = cv::Size(960, 540);
	//color_size = cv::Size(1920, 1080); color_fps = 30; // [TEMPORARY]
	depth_size = cv::Size(640, 480);

	// set multiple streams descriptor (sync)
	PXCVideoModule::DataDesc desc = {}; // set empty at first
	if (sync) {
		desc.receivePartialSample = false;

		auto SetRSStream = [&](PXCCapture::StreamType st, cv::Size size, int fps) {
			desc.streams[st].sizeMax.width  = desc.streams[st].sizeMin.width  = size.width;
			desc.streams[st].sizeMax.height = desc.streams[st].sizeMax.height = size.height;
			desc.streams[st].frameRate.min  = desc.streams[st].frameRate.max  = (float)fps;
			if (sync) desc.streams[st].options = desc.streams[st].options | PXCCapture::Device::STREAM_OPTION_STRONG_STREAM_SYNC;
		};
		SetRSStream(PXCCapture::STREAM_TYPE_COLOR, color_size, color_fps); // [CAUTION] FHD (1920x1080) only permits 30fps, while QHD (960x540) permits 60fps
		SetRSStream(PXCCapture::STREAM_TYPE_DEPTH, depth_size, 60);
		SetRSStream(PXCCapture::STREAM_TYPE_IR   , depth_size, 60);

		sts = psm->EnableStreams(&desc);
		CHECK_STATUS(sts, "Unable to create multiple synced streams");
	}
	else {
		desc.receivePartialSample = true; // async

		sts = psm->EnableStream(PXCCapture::STREAM_TYPE_COLOR, color_size.width, color_size.height, 60);
		CHECK_STATUS(sts, "Unable to create color stream");
		sts = psm->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, depth_size.width, depth_size.height, 60);
		CHECK_STATUS(sts, "Unable to create depth stream");
		sts = psm->EnableStream(PXCCapture::STREAM_TYPE_IR   , depth_size.width, depth_size.height, 60);
		CHECK_STATUS(sts, "Unable to create infrared stream");
	}

	return true;
}
void RSCamera::releaseRSManagers()
{
	// [CAUTION] DO NOT RELEASE HERE: it is reference...
	if (pcm_ref) {
		pcm_ref = nullptr;
	}

	if (psm) {
		psm->Close();
		psm->Release();
		psm = nullptr;
	}
}
