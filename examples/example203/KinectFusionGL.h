/******************************************************************************
New API to utilize KinectFusion routine in GL viewer
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#ifndef KINECT_FUSION_GL
#define KINECT_FUSION_GL

#include <KinectFusionHelperEx.h>

#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <opencv2/core/core.hpp>


///////////////////////////////////////////////////////////////////////////////
// Class for KinectFusion 
///////////////////////////////////////////////////////////////////////////////
class KinectFusionGL
{
public:
	// ctor / dtor
	KinectFusionGL(NUI_FUSION_RECONSTRUCTION_PARAMETERS params);
	KinectFusionGL();
	~KinectFusionGL();

	// initializer / finalizer
	bool InitVolume(NUI_FUSION_RECONSTRUCTION_PARAMETERS params = { 384.0f, 128,128,128 });
	bool InitCamera(int id, const std::vector<float>& camParams4x1, const cv::Size& imageSize);
	void End();

	// File I/O
	bool LoadVolume(const char* xmlFile);

	// visualization - 3D
	void GetVolumeArea(glm::mat4 &worldToVolume, glm::vec3 &minAB, glm::vec3 &maxAB);
	bool GetTriMesh(std::vector<glm::vec3>& V, std::vector<glm::vec3>& N,
		std::vector<glm::u8vec3>& C, std::vector<glm::uint>& F);

	// visualization - 2.5D
	bool RenderVolume(const int id, const glm::mat4& viewGL,
		std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::u8vec3>& colors);

	const NUI_FUSION_RECONSTRUCTION_PARAMETERS GetKinfuParams() { return kinfuParams; };

private:

	// KinectFusion default data
	Matrix4 m_worldToCameraTransform;
	Matrix4 m_worldToVolumeTransform;
	Matrix4 m_defaultWorldToVolumeTransform;
	Matrix4 m_worldToBGRTransform;

	NUI_FUSION_RECONSTRUCTION_PARAMETERS kinfuParams;
	INuiFusionColorReconstruction* m_pVolume = nullptr;

	////////////////////////////////////////////////////////////
	// for visualization
	////////////////////////////////////////////////////////////
	void releaseKinfuImages(int id);

	bool resetVolumeTransform();
	bool recreateVolume();

	bool renderVolume(const glm::mat4& viewGL,
		const NUI_FUSION_IMAGE_FRAME* pointCloudFrame, const NUI_FUSION_IMAGE_FRAME* colorFrame,
		std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::u8vec3>& colors);
	
	NUI_FUSION_IMAGE_FRAME* m_pRaycastPointCloud   [2] = { nullptr, nullptr };
	NUI_FUSION_IMAGE_FRAME* m_pCapturedSurfaceColor[2] = { nullptr, nullptr };
};

#endif
