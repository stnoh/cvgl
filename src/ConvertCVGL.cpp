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
inline glm::vec3 convertDepth2Point(const glm::mat4& proj_inv, glm::vec3 pt_img, float width, float height)
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

std::vector<glm::vec3> ConvertDepthImage2PointCloud(const glm::mat4 proj_inv,
	const cv::Mat& depthImage, const bool full)
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

void ConvertColorDepthImage2PointCloud(const glm::mat4 proj_inv,
	const cv::Mat& colorImage, const cv::Mat& depthImage,
	std::vector<glm::vec3>& points, std::vector<glm::u8vec3>& colors)
{
	points.clear();
	colors.clear();

	// check 8-bit, 3-channel unsigned byte point...
	if (colorImage.type() != CV_8UC3) {
		std::cerr << "ERROR: it only allows 8-bit, 3-channel unsigned byte point image..." << std::endl;
		return;
	}

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


///////////////////////////////////////////////////////////////////////////////
// check vertex visibility using depth information
///////////////////////////////////////////////////////////////////////////////
std::set<glm::uint> GetVisibleVertexIndices(
	glm::mat4 ProjViewModel, const std::vector<glm::vec3>& model_verts,
	const cv::Mat& depthImage, const float threshold)
{
	std::set<glm::uint> v_indices;

	// convert buffer value to camera space Z
	float* depth = (float*)depthImage.data;
	int w = depthImage.cols;
	int h = depthImage.rows;

	for (int vidx = 0; vidx < model_verts.size(); vidx++)
	{
		glm::vec4 pt_proj = ProjViewModel * glm::vec4(model_verts[vidx], 1.0f);
		glm::vec3 pt_NDC = pt_proj / pt_proj.w;

		// filter 1) out of screen
		if (pt_NDC.x < -1.0 || +1.0 < pt_NDC.x ||
			pt_NDC.y < -1.0 || +1.0 < pt_NDC.y) continue;

		// filter 2) compare to rendered depth
		glm::vec2 pt2d = glm::vec2(0.5f * (pt_NDC + 1.0f));
		int i = w * (pt2d.x);
		int j = h * (1.0f - pt2d.y); // [CAUTION] FLIPPED
		float d = depth[i + j * w];
		float pixel_z = 2.0f * d - 1.0f;

		if (glm::length(pt_NDC.z - pixel_z) > threshold) continue;

		v_indices.insert(vidx);
	}

	return v_indices;
}


///////////////////////////////////////////////////////////////////////////////
// convert normal to normalmap color
///////////////////////////////////////////////////////////////////////////////
inline glm::u8vec3 getNormalColor(const glm::vec3& normal)
{
	// coloring is based on "normal = (2*color)-1"
	// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
	glm::vec3 n = glm::normalize(normal);
	glm::vec3 c = 127.5f * (n + glm::vec3(1.0));
	return c;
}
std::vector<glm::u8vec3> GetNormalColors(const std::vector<glm::vec3>& normal)
{
	std::vector<glm::u8vec3> colors;
	for (glm::vec3 _n : normal) {
		colors.push_back(getNormalColor(_n));
	}

	return colors;
}

}
