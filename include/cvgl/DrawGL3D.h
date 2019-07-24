/******************************************************************************
Helper for drawing simple but general 3D objects with GL
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#ifndef DRAW_GL_3D
#define DRAW_GL_3D

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <vector>

namespace cvgl {

// simple primitive
void drawAxes(float length);
void drawGridXZ(float length, int step);

// camera frustum
//void drawPerspCamera(glm::mat4 proj); // no need to expose
//void drawOrthoCamera(glm::mat4 proj); // no need to expose
void drawCameraFrustum(glm::mat4 proj);

// 3D data
void drawPointCloud(const std::vector<glm::vec3>& points);

// lighting
void setLight(GLenum lightNum, glm::vec4 lightPos,
	const glm::vec4 ambient  = glm::vec4(0.1f, 0.1f, 0.1f, 0.1f), 
	const glm::vec4 diffuse  = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
	const glm::vec4 specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) );
}

#endif
