/******************************************************************************
C++ facade class for Kinect v2 API with OpenCV & glm containers
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include <cvgl/Kinect2.h>

using namespace cvgl;

#define CHECK_ERROR(hr, msg) if (FAILED(hr)) { fprintf(stderr, "ERROR: %s.\n", msg); return false; }


///////////////////////////////////////////////////////////////////////////////
// ctor / dtor
///////////////////////////////////////////////////////////////////////////////
Kinect2::Kinect2()
{
	// initialize OpenCV image containers
	ResizeImageContainers();
}
Kinect2::~Kinect2()
{
	End(); // for safety
}


///////////////////////////////////////////////////////////////////////////////
// initialize / finalize
///////////////////////////////////////////////////////////////////////////////
bool Kinect2::Init()
{
	End(); // for safety

	BOOLEAN isOpen = FALSE;

	CHECK_ERROR(GetDefaultKinectSensor(&pSensor), "cannot find default Kinect2");
	CHECK_ERROR(pSensor->Open(), "cannot initialize Kinect2");
	CHECK_ERROR(pSensor->get_IsOpen(&isOpen), "cannot open Kinect2");
	if (!isOpen) {
		fprintf(stderr, "ERROR: cannot open Kinect2.\n");
		return false;
	}

	CHECK_ERROR(pSensor->get_CoordinateMapper(&pCoordinateMapper), "get_CoordinateMapper");

	if (!initRawStream()) {
		fprintf(stderr, "ERROR: cannot initialize streams.\n");
		return false;
	}

	ResizeImageContainers();

	return true;
}
void Kinect2::End()
{
	if (NULL != pColorFrameReader) pColorFrameReader->Release();
	if (NULL != pDepthFrameReader) pDepthFrameReader->Release();
	if (NULL != pIrFrameReader   ) pIrFrameReader->Release();

	if (NULL != pCoordinateMapper) pCoordinateMapper->Release();

	if (NULL != pSensor) {
		pSensor->Close();
		pSensor->Release();
	}
}


///////////////////////////////////////////////////////////////////////////////
// common update routine
///////////////////////////////////////////////////////////////////////////////
bool Kinect2::Update()
{
	if (!updateColorImage()) return false;
	if (!updateDepthImage()) return false;
	if (!updateIrImage())    return false;

	//return RGBDCamera::Update(); // ***************************************** [TODO]

	// [TEMPORARY] update point cloud by Kinect's routine
	static bool get_calib = false;
	if (!get_calib)
	{
		initKinect2Setting();
		get_calib = true;
	}

	return update3DInfo();
}

void Kinect2::ResizeImageContainers()
{
	RGBDCamera::ResizeImageContainers();

	color_image = cv::Mat(color_size, CV_8UC4); // Kinect SDK uses BGRA format
}


///////////////////////////////////////////////////////////////////////////////
// camera-dependent APIs: methods for initialization
///////////////////////////////////////////////////////////////////////////////
bool Kinect2::initRawStream()
{
	if (!initColorImage()) {
		fprintf(stderr, "ERROR: cannot initialize color image stream.\n");
		return false;
	}
	if (!initDepthImage()) {
		fprintf(stderr, "ERROR: cannot initialize depth image stream.\n");
		return false;
	}
	if (!initIrImage()) {
		fprintf(stderr, "ERROR: cannot initialize infrared image stream.\n");
		return false;
	}

	return true;
}

bool Kinect2::initColorImage()
{
	IColorFrameSource* pColorFrameSource = NULL;
	CHECK_ERROR(pSensor->get_ColorFrameSource(&pColorFrameSource), "cannot initialize color frame");
	CHECK_ERROR(pColorFrameSource->OpenReader(&pColorFrameReader), "cannot acquire color frame reader");

	IFrameDescription* colorFrameDescription;
	CHECK_ERROR(pColorFrameSource->CreateFrameDescription(ColorImageFormat::ColorImageFormat_Bgra, &colorFrameDescription), "cannot create color frame description");

	int width, height;
	colorFrameDescription->get_Width(&width);
	colorFrameDescription->get_Height(&height);
//	colorFrameDescription->get_BytesPerPixel(&bytesPerPixel); // [DEPRECATED]
	color_size = cv::Size(width, height);

	return true;
}
bool Kinect2::initDepthImage()
{
	IDepthFrameSource* pDepthFrameSource = NULL;
	CHECK_ERROR(pSensor->get_DepthFrameSource(&pDepthFrameSource), "cannot initialize depth frame");
	CHECK_ERROR(pDepthFrameSource->OpenReader(&pDepthFrameReader), "cannot acquire depth frame reader");

	IFrameDescription* depthFrameDescription;
	CHECK_ERROR(pDepthFrameSource->get_FrameDescription(&depthFrameDescription), "cannot acquire depth frame description");

	int width, height;
	depthFrameDescription->get_Width(&width);
	depthFrameDescription->get_Height(&height);
//	depthFrameDescription->get_BytesPerPixel(&bytesPerPixel); // [DEPRECATED]
	depth_size = cv::Size(width, height);

	return true;
}
bool Kinect2::initIrImage()
{
	IInfraredFrameSource* pIrFrameSource = NULL;
	CHECK_ERROR(pSensor->get_InfraredFrameSource(&pIrFrameSource), "cannot initialize infrared frame");
	CHECK_ERROR(pIrFrameSource->OpenReader(&pIrFrameReader), "cannot acquire infrared frame reader");

	IFrameDescription* irFrameDescription;
	CHECK_ERROR(pIrFrameSource->get_FrameDescription(&irFrameDescription), "cannot acquire infrared frame description");

	// [DEPRECATED] totally same with depth image resolution
	/*
	int width, height;
	irFrameDescription->get_Width(&width);
	irFrameDescription->get_Height(&height);
	irFrameDescription->get_BytesPerPixel(&bytesPerPixel);
	//*/

	return true;
}

bool Kinect2::initKinect2Setting()
{
	// [TEMPORARY] update depth camera matrix due to Kinect2 API limitation
	CameraIntrinsics intrinsics;
	pCoordinateMapper->GetDepthCameraIntrinsics(&intrinsics);

	fx_d = intrinsics.FocalLengthX;
	fy_d = intrinsics.FocalLengthY;
	cx_d = intrinsics.PrincipalPointX;
	cy_d = intrinsics.PrincipalPointY;

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// camera-dependent APIs: method for update
///////////////////////////////////////////////////////////////////////////////
bool Kinect2::updateColorImage()
{
	bool updated = false;

	IColorFrame* pColorFrame;

	HRESULT hr = pColorFrameReader->AcquireLatestFrame(&pColorFrame);
	if (SUCCEEDED(hr)) {
		hr = pColorFrame->CopyConvertedFrameDataToArray(
			color_size.area() * 4 * sizeof(unsigned char),
			(BYTE*)color_image.data, ColorImageFormat::ColorImageFormat_Bgra);
		if (SUCCEEDED(hr)) updated = true;
	}

	if (NULL != pColorFrame) pColorFrame->Release();
	pColorFrame = NULL;

	return updated;
}
bool Kinect2::updateDepthImage()
{
	bool updated = false;

	IDepthFrame* pDepthFrame;

	HRESULT hr = pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);
	if (SUCCEEDED(hr)) {
		hr = pDepthFrame->CopyFrameDataToArray(
			static_cast<UINT>(depth_size.area()), (USHORT*)depth_image.data);
		if (SUCCEEDED(hr)) updated = true;
	}

	if (NULL != pDepthFrame) pDepthFrame->Release();
	pDepthFrame = NULL;

	return updated;
}
bool Kinect2::updateIrImage()
{
	bool updated = false;

	IInfraredFrame* pIrFrame;

	HRESULT hr = pIrFrameReader->AcquireLatestFrame(&pIrFrame);
	if (SUCCEEDED(hr)) {
		hr = pIrFrame->CopyFrameDataToArray(
			static_cast<UINT>(depth_size.area()), (USHORT*)ir_image.data);
		if (SUCCEEDED(hr)) updated = true;
	}

	if (NULL != pIrFrame) pIrFrame->Release();
	pIrFrame = NULL;

	return updated;
}

bool Kinect2::update3DInfo()
{
	HRESULT hr;

	int numOfV = depth_size.area();

	// depth image to point cloud
	std::vector<CameraSpacePoint> cameraSpacePoints(numOfV);
	hr = pCoordinateMapper->MapDepthFrameToCameraSpace(
		numOfV,
		(unsigned short*)depth_image.data,
		numOfV,
		&cameraSpacePoints[0]);
	if (FAILED(hr)) return false;

	// color mapping on point cloud
	std::vector<ColorSpacePoint> colorSpacePoints(numOfV);
	hr = pCoordinateMapper->MapDepthFrameToColorSpace(
		numOfV,
		(unsigned short*)depth_image.data,
		numOfV,
		&colorSpacePoints[0]);
	if (FAILED(hr)) return false;

	// clear colored layer as black
	memset(colored_layer.data, 0, sizeof(unsigned char) * 4 * numOfV);

	for (int vidx = 0; vidx < numOfV; vidx++) {
		CameraSpacePoint p = cameraSpacePoints[vidx];
		ColorSpacePoint uv = colorSpacePoints[vidx];
		int ci = (int)(uv.X);
		int cj = (int)(uv.Y);

		// get point color
		glm::u8vec3 c = glm::u8vec3(128, 128, 128); // fallback as gray
		if (0 <= ci && ci <= color_size.width - 1 &&
			0 <= cj && cj <= color_size.height - 1) {
			int pidx = ci + cj * color_size.width;

			c.b = color_image.data[4 * pidx + 0];
			c.g = color_image.data[4 * pidx + 1];
			c.r = color_image.data[4 * pidx + 2];
			const unsigned char a = 0;

			// write BGRA color at once
			int bgra = (a << 24 | c.r << 16 | c.g << 8 | c.b);
			((int*)colored_layer.data)[vidx] = bgra;
		}
		else {
			// delete non-colored depth pixel for KinectFusion
			((unsigned short*)depth_image.data)[vidx] = 0;
		}

		points[vidx] = glm::vec3(p.X, p.Y, -p.Z); // [CAUTION] DX to GL ******* [TODO] check later
		colors[vidx] = c;
	}

	return true;
}
