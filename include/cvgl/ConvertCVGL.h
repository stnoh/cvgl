/******************************************************************************
Color image conversion between OpenGL and OpenCV
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#pragma once

#ifndef CONVERT_CV_GL
#define CONVERT_CV_GL

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <opencv2/imgproc/imgproc.hpp>

#include <set> // for GetVisibleVertexIndices()

namespace cvgl {

// copy GL framebuffer to image (cv::Mat) (from GPU to CPU)
cv::Mat GetRenderedColorImage8U (int w, int h);
cv::Mat GetRenderedColorImage32F(int w, int h);
cv::Mat GetRenderedDepthImage32F(int w, int h);

// convert depth image (cv::Mat) to point cloud (in GL coordinate)
//inline glm::vec3 convertDepth2Point(const glm::mat4& proj_inv, glm::vec3 pt_img, float width, float height); // no need to expose
std::vector<glm::vec3> ConvertDepthImage2PointCloud(const glm::mat4 proj_inv,
	const cv::Mat& depthImage, const bool full = false);
void ConvertColorDepthImage2PointCloud(const glm::mat4 proj_inv,
	const cv::Mat& colorImage, const cv::Mat& depthImage,
	std::vector<glm::vec3>& points, std::vector<glm::u8vec3>& colors);

// check vertex visibility using depth information
std::set<glm::uint> GetVisibleVertexIndices(
	glm::mat4 ProjViewModel, const std::vector<glm::vec3>& model_verts,
	const cv::Mat& depthImage, const float threshold = 1e-3);

// convert normal to normalmap color
//inline glm::u8vec3 getNormalColor(const glm::vec3& normal); // no need to expose
std::vector<glm::u8vec3> GetNormalColors(const std::vector<glm::vec3>& normal);

}

#endif
