/******************************************************************************
Helper for OpenCV image handling with GL
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#ifndef DRAW_GL_2D
#define DRAW_GL_2D

#include <GL/glew.h>
#include <opencv2/opencv.hpp>

namespace cvgl {

void drawImage(const cv::Mat& img, cv::Rect viewportGL, const double a = 1.0, const double b = 0.0);

}

#endif
