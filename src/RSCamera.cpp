/******************************************************************************
C++ facade class for RealSense camera API with OpenCV containers
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include "cvgl/RSCamera.h"
#include <RealSense/Session.h>
#include <fstream>

#define CHECK_STATUS(sts, msg) if (sts < PXC_STATUS_NO_ERROR) { fprintf(stderr, "ERROR: %s.\n", msg); return false; }

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
}
RSCamera::~RSCamera()
{
	End(); // for safety
	if (session) session->Release();
}


///////////////////////////////////////////////////////////////////////////////
// initialize/finalize
///////////////////////////////////////////////////////////////////////////////
bool RSCamera::Init()
{
	releaseRSManagers(); // for safety

	// create PXCSenseManager
	psm = session->CreateSenseManager();
	if (!psm) {
		fprintf(stderr, "ERROR: Unable to create the PXCSenseManager.\n");
		End();
		return false;
	}

	// get PXCCaptureManager as "reference"
	pcm_ref = psm->QueryCaptureManager();
	if (!pcm_ref) {
		fprintf(stderr, "Unable to query PXCCaptureManager.\n");
		End();
		return false;
	}

	initRawStreams(true);
	ResizeImageContainers();

	// start streaming
	pxcStatus sts = psm->Init();
	if (sts < PXC_STATUS_NO_ERROR) {
		fprintf(stderr, "ERROR: Unable to create the PXCCaptureManager.\n");
		End();
		return false;
	}

	// after starting the streaming ...
	initRSCameraSetting();
	ReadCalibrationData();

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

	// run 3D update in super class
	return RGBDCamera::Update();
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
bool RSCamera::initRSCameraSetting()
{
	PXCCapture::Device *device = pcm_ref->QueryDevice(); // [CAUTION] it is reference: DO NOT RELEASE

	////////////////////////////////////////////////////////////
	// color camera settings
	////////////////////////////////////////////////////////////

	// exposure (=shutter speed)
//	CHECK_STATUS(device->SetColorExposure(-5), "cannot set this exposure"); // [-8:-2]
	CHECK_STATUS(device->SetColorAutoExposure(true), "cannot set auto exposure");

//	CHECK_STATUS(device->SetColorGain(96), "cannot set this gain"); // gain range = [0:128], default = 64
//	CHECK_STATUS(device->SetColorSharpness(100), "cannot set this sharpness"); // sharpness range = [0:100], default = 50

//	CHECK_STATUS(device->SetColorWhiteBalance(4600), "cannot set this white balance"); // temperature = [2800:6500] Kelvin
	CHECK_STATUS(device->SetColorAutoWhiteBalance(true), "cannot set auto whitebalance as true");

	////////////////////////////////////////////////////////////
	// [TEMPORARY] get factory calibration from camera API
	////////////////////////////////////////////////////////////
#if 1
	if (!std::ifstream(calibXML).good())
	{
		PXCProjection *projection = device->CreateProjection();
		Intel::RealSense::Calibration *calib = projection->QueryCalibration();

		auto GetCameraMatrix = [](PXCCalibration::StreamCalibration camCalibData)->cv::Mat
		{
			cv::Mat cameraMatrix = cv::Mat::zeros(3, 3, CV_64F);
			cameraMatrix.at<double>(cv::Point(0, 0)) = camCalibData.focalLength.x;
			cameraMatrix.at<double>(cv::Point(1, 1)) = camCalibData.focalLength.y;
			cameraMatrix.at<double>(cv::Point(2, 0)) = camCalibData.principalPoint.x;
			cameraMatrix.at<double>(cv::Point(2, 1)) = camCalibData.principalPoint.y;
			cameraMatrix.at<double>(cv::Point(2, 2)) = 1.0f;

			return cameraMatrix;
		};

		auto GetDistCoeffs = [](PXCCalibration::StreamCalibration camCalibData)->cv::Mat
		{
			cv::Mat distCoeffs = cv::Mat::zeros(5, 1, CV_64F);
			distCoeffs.at<double>(0) = camCalibData.radialDistortion[0];
			distCoeffs.at<double>(1) = camCalibData.radialDistortion[1];
			distCoeffs.at<double>(2) = camCalibData.tangentialDistortion[0];
			distCoeffs.at<double>(3) = camCalibData.tangentialDistortion[1];
			distCoeffs.at<double>(4) = camCalibData.radialDistortion[2];

			return distCoeffs;
		};

		cv::FileStorage fs(calibXML, cv::FileStorage::WRITE);

		// depth (id=0): intrinsic parameters only, no extrinsic (origin)
		{
			PXCCalibration::StreamCalibration camCalibData;
			PXCCalibration::StreamTransform   camTransform;
			calib->QueryStreamProjectionParameters(PXCCapture::STREAM_TYPE_DEPTH, &camCalibData, &camTransform);

			PXCPointF32 focal = device->QueryDepthFocalLength();

			fs << "depth_camera_matrix" << GetCameraMatrix(camCalibData);
			fs << "depth_dist_coeffs"   << GetDistCoeffs(camCalibData);
			fs << "depth_image_size"    << depth_size;
			fs << "depth_camera_focal"  << focal.x; // x == y

			fs << "depth_clipping_near" << 200;  //  200 [mm]
			fs << "depth_clipping_far"  << 1200; // 1200 [mm]
		}

		// color (id=1): intrinsic/extrinsic parameters
		{
			PXCCalibration::StreamCalibration camCalibData;
			PXCCalibration::StreamTransform   camTransform;
			calib->QueryStreamProjectionParameters(PXCCapture::STREAM_TYPE_COLOR, &camCalibData, &camTransform);

			PXCPointF32 focal = device->QueryColorFocalLength();

			fs << "color_camera_matrix" << GetCameraMatrix(camCalibData);
			fs << "color_dist_coeffs"   << GetDistCoeffs(camCalibData);
			fs << "color_image_size"    << color_size;
			fs << "color_camera_focal"  << focal.x; // x == y

			cv::Mat rMat = cv::Mat(3, 3, CV_64F);
			for (int j = 0; j < 3; j++)
			for (int i = 0; i < 3; i++){
				rMat.at<double>(cv::Point(j, i)) = camTransform.rotation[i][j];
			}
			cv::Vec3f   tVec(camTransform.translation);

			fs << "depth_to_color_rmat" << rMat;
			fs << "depth_to_color_tvec" << tVec;
		}

		projection->Release();
		projection = nullptr;
	}
#endif
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
