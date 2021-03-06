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

// simple primitive (GLU)
void drawAxes(float length);
void drawSphere(float radius, glm::vec4 color);

// simple primitive (GL_LINES)
void drawGridXZ(float length, int step);
void drawAABB(glm::vec3 minAB, glm::vec3 maxAB);

// camera frustum
//void drawPerspCamera(glm::mat4 proj); // no need to expose
//void drawOrthoCamera(glm::mat4 proj); // no need to expose
void drawCameraFrustum(glm::mat4 proj);

// 3D data
void drawPointCloud(const std::vector<glm::vec3>& points);
void drawPointCloud(const std::vector<glm::vec3>& points, const std::vector<glm::u8vec3>& colors);

void drawTriMesh(const std::vector<glm::vec3>& V, const std::vector<glm::uint>& F);
void drawTriMesh(const std::vector<glm::vec3>& V, const std::vector<glm::vec3>& N, const std::vector<glm::uint>& F);
void drawTriMesh(const std::vector<glm::vec3>& V, const std::vector<glm::u8vec3>& C, const std::vector<glm::uint>& F);
void drawTriMesh(const std::vector<glm::vec3>& V, const std::vector<glm::vec3>& N, const std::vector<glm::u8vec3>& C, const std::vector<glm::uint>& F);

// lighting
void setLight(GLenum lightNum, glm::vec4 lightPos,
	const glm::vec4 ambient  = glm::vec4(0.1f, 0.1f, 0.1f, 0.1f), 
	const glm::vec4 diffuse  = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
	const glm::vec4 specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) );
}

#endif
