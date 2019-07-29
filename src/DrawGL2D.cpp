/******************************************************************************
Helper for OpenCV image handling with GL
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include "cvgl/DrawGL2D.h"
#include <glm/ext.hpp>

namespace cvgl {

///////////////////////////////////////////////////////////////////////////////
// draw cv::Mat in OpenGL viewport
///////////////////////////////////////////////////////////////////////////////
void drawImage(const cv::Mat& img, const double a, const double b)
{
	int width  = img.cols;
	int height = img.rows;

	GLenum format = GL_BGR;

	int channels = img.channels();
	switch (channels) {
	case 1: format = GL_LUMINANCE; break;
	case 2: format = GL_LUMINANCE_ALPHA; break;
	case 3: format = GL_BGR; break;
	case 4: format = GL_BGRA; break;
	default: fprintf(stderr, "invalid pixel channels.\n"); return;
	}

	int depth = img.depth();

	// check image type
	/*
	int type = GL_UNSIGNED_BYTE;
	switch (depth) {
	case CV_8U : type = GL_UNSIGNED_BYTE; break;
	case CV_8S : type = GL_BYTE; break;
	case CV_16U: type = GL_UNSIGNED_SHORT; break;
	case CV_16S: type = GL_SHORT; break;
	case CV_32S: type = GL_INT; break;
	case CV_32F: type = GL_FLOAT; break;
	case CV_64F: type = GL_DOUBLE; break;
	default: fprintf(stderr, "invalid pixel depth.\n"); break;
	}
	//*/

	if (1.0 == a && 0.0 == b && depth == CV_8U)
	{
		// in case of unsigned byte: "direct" drawing
		glDrawPixels(width, height, format, GL_UNSIGNED_BYTE, img.data);
	}
	else
	{
		// in case of different format
		int new_type = CV_MAKETYPE(CV_8U, img.channels());
		cv::Mat img_8u(img.cols, img.rows, new_type);
		img.convertTo(img_8u, new_type, a, b);
		glDrawPixels(width, height, format, GL_UNSIGNED_BYTE, img_8u.data);
	}
}

void drawImage(const cv::Mat& img, cv::Rect viewportGL, const double a, const double b)
{
	cv::Size imgSize = cv::Size(img.cols, img.rows);
	if (0 == imgSize.width || 0 == imgSize.height) return; // fast return

	// set view-projection matrices with immediate model
	float width = (float)viewportGL.width; float height = (float)viewportGL.height;

	glm::mat4 proj = glm::ortho( 0.0f, width, 0.0f, height );
	glMatrixMode(GL_PROJECTION); glLoadIdentity(); glLoadMatrixf(glm::value_ptr(proj));
	glMatrixMode(GL_MODELVIEW);  glLoadIdentity();

	// image flipping by renderer setting
	glRasterPos2f(0.0f, height - 1.0f); // [CAUTION] FLIPPED
	float zoom_x = (float)width  / (float)imgSize.width;
	float zoom_y = (float)height / (float)imgSize.height;
	glPixelZoom(zoom_x, -zoom_y); // [CAUTION] FLIPPED
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	drawImage(img, a, b);
}

}
