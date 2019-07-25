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

namespace cvgl {

// copy GL framebuffer to image (cv::Mat) (from GPU to CPU)
cv::Mat GetRenderedColorImage8U (int w, int h);
cv::Mat GetRenderedColorImage32F(int w, int h);
cv::Mat GetRenderedDepthImage32F(int w, int h);

// convert depth image (cv::Mat) to point cloud (in GL coordinate)
//inline glm::vec3 convertDepth2Point(glm::mat4 proj_inv, glm::vec3 pt_img, float width, float height); // no need to expose
std::vector<glm::vec3> ConvertDepthImage2PointCloud(glm::mat4 proj_inv, cv::Mat depthImage, bool full = false);
void ConvertColorDepthImage2PointCloud(glm::mat4 proj_inv, cv::Mat colorImage, cv::Mat depthImage,
	std::vector<glm::vec3>& points, std::vector<glm::u8vec3>& colors);

// convert normal to normalmap color
//inline glm::u8vec3 getNormalColor(glm::vec3 normal); // no need to expose
std::vector<glm::u8vec3> GetNormalColors(const std::vector<glm::vec3>& normal);

}

#endif
