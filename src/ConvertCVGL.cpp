/******************************************************************************
Color image conversion between OpenGL and OpenCV
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include "cvgl/ConvertCVGL.h"
#include <iostream>

namespace cvgl {

///////////////////////////////////////////////////////////////////////////////
// copy GL framebuffer to image (cv::Mat) (from GPU to CPU)
///////////////////////////////////////////////////////////////////////////////
cv::Mat GetRenderedColorImage8U(int w, int h)
{
	cv::Mat colorBuffer = cv::Mat(h, w, CV_8UC3); // compatible with GL_RGB
	glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, colorBuffer.data);

	// format change from GL to CV
	cv::cvtColor(colorBuffer, colorBuffer, CV_RGB2BGR);
	cv::flip(colorBuffer, colorBuffer, 0);

	return colorBuffer;
}
cv::Mat GetRenderedColorImage32F(int w, int h)
{
	cv::Mat colorBuffer = cv::Mat(h, w, CV_32FC3); // compatible with GL_RGB
	glReadPixels(0, 0, w, h, GL_RGB, GL_FLOAT, colorBuffer.data);

	// format change from GL to CV
	cv::cvtColor(colorBuffer, colorBuffer, CV_RGB2BGR);
	cv::flip(colorBuffer, colorBuffer, 0);

	return colorBuffer;
}
cv::Mat GetRenderedDepthImage32F(int w, int h)
{
	cv::Mat depthBuffer = cv::Mat(h, w, CV_32FC1); // compatible with GL_DEPTH_COMPONENT32
	glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, depthBuffer.data);
	cv::flip(depthBuffer, depthBuffer, 0);

	// convert buffer value to camera space Z
	float* depth = (float*)depthBuffer.data; // [0.0:1.0]
	for (int j = 0; j < h; j++)
	for (int i = 0; i < w; i++)
	{
		float z_b = depth[i + j*w];
		depth[i + j*w] = (1.0f == z_b) ? 0.0f : z_b; // <inf> -> 0.0
	}

	return depthBuffer;
}


///////////////////////////////////////////////////////////////////////////////
// convert depth image (cv::Mat) to point cloud (in GL coordinate)
// similar to glm::unProject, but it provides more efficient routine
///////////////////////////////////////////////////////////////////////////////
inline glm::vec3 convertDepth2Point(glm::mat4 proj_inv, glm::vec3 pt_img, float width, float height)
{
	// normalize point on 2D image plane
	pt_img.x = (pt_img.x + 0.5f) / width;
	pt_img.y = (height - (pt_img.y + 0.5f)) / height; // [CAUTION] FLIPPED

	// conver to NDC: ( [-1.0:+1.0], [-1.0:+1.0], [-1.0:+1.0] )
	glm::vec4 pt_NDC = glm::vec4(2.0f * pt_img - 1.0f, 1.0f);
	glm::vec4 pt3d = proj_inv * pt_NDC;

	// perspective-aware
	return pt3d / pt3d.w;
}

std::vector<glm::vec3> ConvertDepthImage2PointCloud(glm::mat4 proj_inv, cv::Mat depthImage, bool full)
{
	// check 32-bit, 1-channel floating point...
	if (depthImage.type() != CV_32FC1) {
		std::cerr << "ERROR: it only allows 32-bit, 1-channel floating point image..." << std::endl;
		return std::vector<glm::vec3>();
	}

	int w = depthImage.cols;
	int h = depthImage.rows;

	std::vector<glm::vec3> cloud;

	// convert buffer value to camera space Z
	float* depth = (float*)depthImage.data;

	if (full) {
		// same with image size
		cloud = std::vector<glm::vec3>(w*h);

		for (int j = 0; j < h; j++)
		for (int i = 0; i < w; i++)
		{
			float d = depth[i + j * w];
			glm::vec3 pt2d(i, j, d);
			cloud[i + j * w] = convertDepth2Point(proj_inv, pt2d, w, h);
		}
	}
	else {
		for (int j = 0; j < h; j++)
		for (int i = 0; i < w; i++)
		{
			float d = depth[i + j * w];
			if (0.0f == d) continue; // filter un-rendered pixel

			glm::vec3 pt2d(i, j, d);
			cloud.push_back(convertDepth2Point(proj_inv, pt2d, w, h));
		}
	}

	return cloud;
}

void ConvertColorDepthImage2PointCloud(glm::mat4 proj_inv, cv::Mat colorImage, cv::Mat depthImage,
	std::vector<glm::vec3>& points, std::vector<glm::u8vec3>& colors)
{
	points.clear();
	colors.clear();

	// check 32-bit, 1-channel floating point...
	if (depthImage.type() != CV_32FC1) {
		std::cerr << "ERROR: it only allows 32-bit, 1-channel floating point image..." << std::endl;
		return;
	}

	// color and depth image size should be same...
	if (colorImage.size() != depthImage.size()) {
		std::cerr << "ERROR: color and depth image size does not match ..." << std::endl;
		return;
	}

	int w = depthImage.cols;
	int h = depthImage.rows;

	// convert buffer value to camera space Z
	float* depth = (float*)depthImage.data;

	for (int j = 0; j < h; j++)
	for (int i = 0; i < w; i++)
	{
		float d = depth[i + j * w];
		if (0.0f == d) continue; // filter un-rendered pixel

		glm::vec3 pt2d(i, j, d);
		points.push_back(convertDepth2Point(proj_inv, pt2d, w, h));

		glm::u8vec3 c;
		c.b = colorImage.data[3 * (i + j * w) + 0];
		c.g = colorImage.data[3 * (i + j * w) + 1];
		c.r = colorImage.data[3 * (i + j * w) + 2];
		colors.push_back(c);
	}

	return;
}

}
