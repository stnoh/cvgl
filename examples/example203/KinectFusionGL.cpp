/******************************************************************************
New API to utilize KinectFusion routine in GL viewer
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include "KinectFusionGL.h"
#include "KinectFusionHelperEx.h"

#include <iostream>


///////////////////////////////////////////////////////////////////////////////
// common utilities
///////////////////////////////////////////////////////////////////////////////
#define CHECK_ERROR(hr, msg) if (FAILED(hr)) { fprintf(stderr, "ERROR: %s.\n", msg); return false; }

#define SAFE_FUSION_RELEASE_IMAGE_FRAME(p) { if (p) { static_cast<void>(NuiFusionReleaseImageFrame(p)); (p)=NULL; } }

// Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

void SetIdentityMatrix(Matrix4 &mat)
{
	mat.M11 = 1; mat.M12 = 0; mat.M13 = 0; mat.M14 = 0;
	mat.M21 = 0; mat.M22 = 1; mat.M23 = 0; mat.M24 = 0;
	mat.M31 = 0; mat.M32 = 0; mat.M33 = 1; mat.M34 = 0;
	mat.M41 = 0; mat.M42 = 0; mat.M43 = 0; mat.M44 = 1;
}


///////////////////////////////////////////////////////////////////////////////
// ctor / dtor
///////////////////////////////////////////////////////////////////////////////
KinectFusionGL::KinectFusionGL(NUI_FUSION_RECONSTRUCTION_PARAMETERS params)
{
	SetIdentityMatrix(m_worldToCameraTransform);
	SetIdentityMatrix(m_worldToVolumeTransform);
	SetIdentityMatrix(m_defaultWorldToVolumeTransform);

	kinfuParams = params;

	recreateVolume();
}
KinectFusionGL::KinectFusionGL()
: KinectFusionGL(NUI_FUSION_RECONSTRUCTION_PARAMETERS{ 384.0f, 128,128,128 })
{

}
KinectFusionGL::~KinectFusionGL()
{
	End(); // for safety
}


///////////////////////////////////////////////////////////////////////////////
// initialize / finalize
///////////////////////////////////////////////////////////////////////////////
bool KinectFusionGL::InitVolume(NUI_FUSION_RECONSTRUCTION_PARAMETERS params)
{
	HRESULT hr;

	////////////////////////////////////////////////////////////
	// check KinectFusion parameters
	////////////////////////////////////////////////////////////
	const float recons_limit = 1000.0f;
	if (recons_limit < params.voxelsPerMeter) {
		fprintf(stderr, "WARNING: too dense vpm = %f...\n", params.voxelsPerMeter);
		params.voxelsPerMeter = 1000.0f;
	}

	bool invalid_size = false;
	if (32 > params.voxelCountX) { invalid_size = true; params.voxelCountX = 32; }
	if (32 > params.voxelCountY) { invalid_size = true; params.voxelCountY = 32; }
	if (32 > params.voxelCountZ) { invalid_size = true; params.voxelCountZ = 32; }
	if (invalid_size) fprintf(stderr, "WARNING: minimum voxel in each dimension is 32.\n"); ;

	kinfuParams = params;

	SetIdentityMatrix(m_worldToCameraTransform);
	SetIdentityMatrix(m_worldToVolumeTransform);

	////////////////////////////////////////////////////////////
	// show re-init voxel dimension
	////////////////////////////////////////////////////////////
	int voX = kinfuParams.voxelCountX;
	int voY = kinfuParams.voxelCountY;
	int voZ = kinfuParams.voxelCountZ;
	float vpm = kinfuParams.voxelsPerMeter;
	printf("volume = [ %d x %d x %d ], voxel/meter = %f\n", voX, voY, voZ, vpm);

	// release & create instance with new parameters
	if (nullptr != m_pVolume) CHECK_ERROR(m_pVolume->Release(), "cannot release KinectFusion instance");
	hr = NuiFusionCreateColorReconstruction(&kinfuParams,
		NUI_FUSION_RECONSTRUCTION_PROCESSOR_TYPE_AMP, -1, // use any GPU in PC
		&m_worldToCameraTransform, // initial camera pose
		&m_pVolume); // instance of KinectFusion
	CHECK_ERROR(hr, "cannot create KinectFusion instance");

	return true;
}

bool KinectFusionGL::InitCamera(const std::vector<float>& depthCamParams4x1,
	const cv::Size& depthImageSize, const float clipMin, const float clipMax)
{
	HRESULT hr = S_OK;

	releaseKinfuImages(); // for safety

	// set depth camera information
	this->depthImageSize = depthImageSize;
	this->depthCameraParams = NUI_FUSION_CAMERA_PARAMETERS{
		depthCamParams4x1[0], depthCamParams4x1[1],
		depthCamParams4x1[2], depthCamParams4x1[3]
	};

	if (!createKinfuImages()) {
		fprintf(stderr, "ERROR: cannot initialize KinectFusion image containers.\n");
		return false;
	}

	minDepthClip = clipMin;
	maxDepthClip = clipMax;

	return true;
}
bool KinectFusionGL::createKinfuImages()
{
	HRESULT hr = S_OK;

	int width  = depthImageSize.width;
	int height = depthImageSize.height;

	for (int i = 0; i < 2; i++)
	{
		hr = NuiFusionCreateImageFrame(NUI_FUSION_IMAGE_TYPE_POINT_CLOUD,
			width, height, &depthCameraParams, &m_pRaycastPointCloud[i]);
		CHECK_ERROR(hr, "cannot create image frame");

		hr = NuiFusionCreateImageFrame(NUI_FUSION_IMAGE_TYPE_COLOR,
			width, height, &depthCameraParams, &m_pCapturedSurfaceColor[i]);
		CHECK_ERROR(hr, "cannot create image frame");
	}

	return true;
}

bool KinectFusionGL::resetVolumeTransform()
{
	if (nullptr == m_pVolume) return false;

	HRESULT hr = S_OK;
	SetIdentityMatrix(m_worldToCameraTransform);

	// the center of volume is at the world's origin
	Matrix4 worldToVolumeTransform = m_defaultWorldToVolumeTransform;
	worldToVolumeTransform.M43 += 0.5 * kinfuParams.voxelCountZ;

	// translate camera to Z-direction
	const float minDist = 0.15f;
	m_worldToCameraTransform.M43 += 0.5f * kinfuParams.voxelCountZ / kinfuParams.voxelsPerMeter;
	m_worldToCameraTransform.M43 += minDist;

	// reset transforms
	hr = m_pVolume->ResetReconstruction(&m_worldToCameraTransform, &worldToVolumeTransform);
	CHECK_ERROR(hr, "cannot reset the KinectFusion");
	m_worldToVolumeTransform = worldToVolumeTransform; // assign "changed" volume transformation in the end ...

	return true;
}
bool KinectFusionGL::recreateVolume()
{
	HRESULT hr = S_OK;

	// show reinit voxel dimension
	int   voX = kinfuParams.voxelCountX;
	int   voY = kinfuParams.voxelCountY;
	int   voZ = kinfuParams.voxelCountZ;
	float vpm = kinfuParams.voxelsPerMeter;
	printf("volume = [ %d x %d x %d ], voxel/meter = %f\n", voX, voY, voZ, vpm);

	// initialize camera & volume poses
	SetIdentityMatrix(m_worldToCameraTransform);
	SetIdentityMatrix(m_worldToVolumeTransform); // meaningless, it will be rewritten by GetCurrentWorldToVolumeTransform

	// release previous instance
	if (nullptr != m_pVolume) {
		hr = m_pVolume->Release();
		CHECK_ERROR(hr, "cannot release KinectFusion instance");
	}

	// create instance with new parameters
	hr = NuiFusionCreateColorReconstruction(&kinfuParams,
		NUI_FUSION_RECONSTRUCTION_PROCESSOR_TYPE_AMP, -1, // use any GPU in PC
		&m_worldToCameraTransform, // initial camera pose
		&m_pVolume); // instance of KinectFusion
	CHECK_ERROR(hr, "cannot create KinectFusion instance");

	// get default world-to-volume transformation
	hr = m_pVolume->GetCurrentWorldToVolumeTransform(&m_defaultWorldToVolumeTransform);
	CHECK_ERROR(hr, "cannot get default world-to-volume transformation");

	if (!resetVolumeTransform()) {
		fprintf(stderr, "ERROR: cannot reset KinectFusion volume.\n");
		return false;
	}

	// calculate world-to-BGR transform in advance (?)
	SetIdentityMatrix(m_worldToBGRTransform);
	m_worldToBGRTransform.M11 = kinfuParams.voxelsPerMeter / kinfuParams.voxelCountX;
	m_worldToBGRTransform.M22 = kinfuParams.voxelsPerMeter / kinfuParams.voxelCountY;
	m_worldToBGRTransform.M33 = kinfuParams.voxelsPerMeter / kinfuParams.voxelCountZ;
	m_worldToBGRTransform.M41 = 0.5f;
	m_worldToBGRTransform.M42 = 0.5f;
	m_worldToBGRTransform.M44 = 1.0f;

	return true;
}

void KinectFusionGL::End()
{
	// clean up KinectFusion
	SafeRelease(m_pVolume);

	// clean up images
	releaseKinfuImages();
}
void KinectFusionGL::releaseKinfuImages()
{
	// release image
	SAFE_FUSION_RELEASE_IMAGE_FRAME(m_pRaycastPointCloud[0]);
	SAFE_FUSION_RELEASE_IMAGE_FRAME(m_pRaycastPointCloud[1]);
	SAFE_FUSION_RELEASE_IMAGE_FRAME(m_pCapturedSurfaceColor[0]);
	SAFE_FUSION_RELEASE_IMAGE_FRAME(m_pCapturedSurfaceColor[1]);
}


///////////////////////////////////////////////////////////////////////////////
// File I/O
///////////////////////////////////////////////////////////////////////////////
bool KinectFusionGL::LoadVolume(const char* xmlFile)
{
	HRESULT hr = S_OK;

	if (nullptr == m_pVolume) {
		fprintf(stderr, "ERROR: KinectFusion is not initialized yet.");
		return false;
	}

	KinfuData kinfuData;
	if (!LoadXML(xmlFile, kinfuData)) {
		fprintf(stderr, "ERROR: cannot load XML file - %s\n", xmlFile);
		return false;
	}

	// append camera/volume transform
	m_worldToCameraTransform = kinfuData.worldToCameraTransform;
	m_worldToVolumeTransform = kinfuData.worldToVolumeTransform;

	// append reconstruction (voxel) parameters
	kinfuParams = kinfuData.reconsParams;
	hr = recreateVolume();

	if (FAILED(hr)) {
		fprintf(stderr, "ERROR: cannot recreate volume data.\n");
		return false;
	}

	// append voxel data
	int voX = kinfuParams.voxelCountX;
	int voY = kinfuParams.voxelCountY;
	int voZ = kinfuParams.voxelCountZ;
	unsigned int voxelSize = voX * voY * voZ;
	
	hr = m_pVolume->ImportVolumeBlock(voxelSize * sizeof(short),
		voxelSize * sizeof(int),
		&kinfuData.volumeBlock[0],
		&kinfuData.colorVolumeBlock[0]);

	if (FAILED(hr)) {
		fprintf(stderr, "ERROR: cannot import volume data.\n");
		return false;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// visualization
///////////////////////////////////////////////////////////////////////////////
void KinectFusionGL::GetVolumeArea(glm::mat4 &worldToVolume,
	glm::vec3 &minAB, glm::vec3 &maxAB)
{
	// convert Matrix4 (KinectFusion) to glm::mat4
	glm::mat4 R = glm::toMat4(glm::quat(glm::radians(glm::vec3(180.0f, 0.0f, 0.0f))));
	float x = m_worldToVolumeTransform.M41 / m_worldToVolumeTransform.M11;
	float y = m_worldToVolumeTransform.M42 / m_worldToVolumeTransform.M22;
	float z = m_worldToVolumeTransform.M43 / m_worldToVolumeTransform.M33;
	glm::mat4 T = glm::translate(-glm::vec3(x, y, z));
	worldToVolume = R * T;

	// axis-aligned bounding box --- (0,0,0) to (X,Y,Z)
	float X = kinfuParams.voxelCountX / kinfuParams.voxelsPerMeter;
	float Y = kinfuParams.voxelCountY / kinfuParams.voxelsPerMeter;
	float Z = kinfuParams.voxelCountZ / kinfuParams.voxelsPerMeter;
	minAB = glm::vec3();
	maxAB = glm::vec3(X, Y, Z);
}
bool KinectFusionGL::GetTriMesh(
	std::vector<glm::vec3>& V, std::vector<glm::vec3>& N,
	std::vector<glm::u8vec3>& C, std::vector<glm::uint>& F)
{
	// extract mesh by built-in function in KinectFusion sample
	INuiFusionColorMesh *mesh;
	CHECK_ERROR(m_pVolume->CalculateMesh(1, &mesh), "Cannot get mesh from volume");

	// information in vertex
	const Vector3 *vertices;
	const Vector3 *normals;
	const int* colors;

	CHECK_ERROR(mesh->GetVertices(&vertices), "Cannot get vertices from mesh structure");
	CHECK_ERROR(mesh->GetNormals(&normals), "Cannot get normals from mesh structure");
	CHECK_ERROR(mesh->GetColors(&colors), "Cannot get colors from mesh structure");

	int numOfV = mesh->VertexCount();

	// remove duplicate vertices
	struct Vertex {
		Vector3 p;
		Vector3 n;
		int c;

		bool operator < (const Vertex &rhs) const
		{
			if (p.x < rhs.p.x) return true; if (p.x > rhs.p.x) return false;
			if (p.y < rhs.p.y) return true; if (p.y > rhs.p.y) return false;
			if (p.z < rhs.p.z) return true; if (p.z > rhs.p.z) return false;

			if (n.x < rhs.n.x) return true; if (n.x > rhs.n.x) return false;
			if (n.y < rhs.n.y) return true; if (n.y > rhs.n.y) return false;
			if (n.z < rhs.n.z) return true; if (n.z > rhs.n.z) return false;

			if (c < rhs.c) return true;
			return false;
		}

		bool operator == (const Vertex &rhs) const
		{
			if (p.x == rhs.p.x && p.y == rhs.p.y && p.z == rhs.p.z &&
				n.x == rhs.n.x && n.y == rhs.n.y && n.z == rhs.n.z &&
				c == rhs.c) return true;

			return false;
		}
	};

	std::vector<Vertex> vertices_filtered;

	// [CHECK ME LATER] replace with std::unordered_map
	std::map<Vertex, int> verticesMap;
	std::map<int, int> duplicatedVertexsMap;

	for (int i = 0; i < numOfV; i++) {
		Vertex data;
		data.p = vertices[i];
		data.n = normals[i];
		data.c = colors[i];

		std::map<Vertex, int>::iterator it = verticesMap.find(data);

		if (verticesMap.end() == it)
		{
			int vidx = vertices_filtered.size();
			verticesMap.insert(std::pair<Vertex, int>(data, vidx));
			duplicatedVertexsMap.insert(std::pair<int, int>(i, vidx));
			vertices_filtered.push_back(data);
		}
		else {
			duplicatedVertexsMap.insert(std::pair<int, int>(i, it->second));
		}
	}

	// resizing containers
	numOfV = vertices_filtered.size();
	V.resize(numOfV); N.resize(numOfV); C.resize(numOfV);

	for (int i = 0; i < numOfV; i++) {
		float px = vertices_filtered[i].p.x;
		float py = vertices_filtered[i].p.y;
		float pz = vertices_filtered[i].p.z;
		float nx = vertices_filtered[i].n.x;
		float ny = vertices_filtered[i].n.y;
		float nz = vertices_filtered[i].n.z;

		// convert coordinates (CV to GL)
		V[i] = glm::vec3(+px, -py, -pz);
		N[i] = glm::vec3(+nx, -ny, -nz); // ???

		int c = vertices_filtered[i].c;
		unsigned char b = ((c) & 255);
		unsigned char g = ((c >> 8) & 255);
		unsigned char r = ((c >> 16) & 255);
		C[i] = glm::u8vec3(r, g, b);
	}

	// information in face
	const int* faces;
	mesh->GetTriangleIndices(&faces);

	int numOfF = mesh->TriangleVertexIndexCount(); // already 3x
	F.clear();

	// get the valid index from map
	for (int i = 0; i < numOfF; i += 3) {

		std::map<int, int>::iterator it0 = duplicatedVertexsMap.find(faces[i + 0]);
		std::map<int, int>::iterator it1 = duplicatedVertexsMap.find(faces[i + 1]);
		std::map<int, int>::iterator it2 = duplicatedVertexsMap.find(faces[i + 2]);

		int vidx0 = it0->second;
		int vidx1 = it1->second;
		int vidx2 = it2->second;

		// remove degenerated triangle
		if (vidx0 == vidx1 || vidx0 == vidx2 || vidx1 == vidx2)
		{
			//printf("Warning: this mesh contains degenerated triangle: [%d %d %d]\n", vidx0, vidx1, vidx2);
			continue;
		}

		F.push_back(vidx0);
		F.push_back(vidx1);
		F.push_back(vidx2);
	}

	mesh->Release();
	return true;
}

bool KinectFusionGL::RenderVolume(const int id, const glm::mat4& viewGL,
	std::vector<glm::vec3>& points,
	std::vector<glm::vec3>& normals,
	std::vector<glm::u8vec3>& colors)
{
	if (nullptr == m_pVolume) return false;

	// check in advance
	if (id < 0 || 1 < id) {
		fprintf(stderr, "ERROR: invalid camera id: check your code.\n");
		return false;
	}
	if (nullptr == m_pRaycastPointCloud[id] || nullptr == m_pCapturedSurfaceColor[id]) {
		fprintf(stderr, "ERROR: image container #%d is not initialized: check your code.\n", id);
		return false;
	}

	return renderVolume(viewGL, m_pRaycastPointCloud[id], m_pCapturedSurfaceColor[id],
		points, normals, colors);
}
bool KinectFusionGL::renderVolume(const glm::mat4& viewGL,
	const NUI_FUSION_IMAGE_FRAME* pointCloudFrame, const NUI_FUSION_IMAGE_FRAME* colorFrame,
	std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::u8vec3>& colors)
{
	HRESULT hr = S_OK;
	Matrix4 viewCV = ConvertGL2Kinect(viewGL);

	// ray-cast to TSDF volume area
	hr = m_pVolume->CalculatePointCloud(
		pointCloudFrame,
		colorFrame,
		&viewCV);
	CHECK_ERROR(hr, "cannot render scene");

	// clear containers in advance
	points.clear();
	normals.clear();
	colors.clear();

	// export rendered point cloud with normal and color
	float* pointCloudData = (float*)pointCloudFrame->pFrameBuffer->pBits;
	uchar* colorImageData = (uchar*)colorFrame->pFrameBuffer->pBits;
	int width  = pointCloudFrame->width;
	int height = pointCloudFrame->height;

	// [CAUTION] stacked in CV-order (from left-top to right-bottom)
	for (int j = 0; j < height; j++)
	for (int i = 0; i < width; i++) {
		int vidx = i + j * width;

		float px = pointCloudData[6 * vidx + 0];
		float py = pointCloudData[6 * vidx + 1];
		float pz = pointCloudData[6 * vidx + 2];

		float nx = pointCloudData[6 * vidx + 3];
		float ny = pointCloudData[6 * vidx + 4];
		float nz = pointCloudData[6 * vidx + 5];

		uchar b = colorImageData[4 * vidx + 0];
		uchar g = colorImageData[4 * vidx + 1];
		uchar r = colorImageData[4 * vidx + 2];

		// do not skip for checking in pixel-wise order
		if (isnan(px) || isnan(py) || isnan(pz) || 0.0f == pz) {
			px = py = pz = 0.0f;
			nx = ny = 0.0f;
			nz = 1.0f;
		}

		////////////////////////////////////////////////////////////
		// convert to view space
		////////////////////////////////////////////////////////////
		auto transform = [](const Vector3 &v1, const Matrix4 &worldToCamera)->Vector3
		{
			// Transform the point from the global frame into the local camera frame.
			Vector3 R;
			R.x = worldToCamera.M41 + (worldToCamera.M11 * v1.x) + (worldToCamera.M21 * v1.y) + (worldToCamera.M31 * v1.z);
			R.y = worldToCamera.M42 + (worldToCamera.M12 * v1.x) + (worldToCamera.M22 * v1.y) + (worldToCamera.M32 * v1.z);
			R.z = worldToCamera.M43 + (worldToCamera.M13 * v1.x) + (worldToCamera.M23 * v1.y) + (worldToCamera.M33 * v1.z);

			return R;
		};
		Vector3 p_view = transform(Vector3{ px,py,pz }, viewCV);
		Vector3 n_view = transform(Vector3{ nx,ny,nz }, viewCV);

		glm::vec3 p = glm::vec3(+p_view.x, -p_view.y, -p_view.z);
		glm::vec3 n = glm::vec3(-n_view.x, +n_view.y, +n_view.z); // go "outside" normal

		glm::u8vec3 c = glm::u8vec3(r, g, b);

		points.push_back(p);
		normals.push_back(n);
		colors.push_back(c);
	}

	return true;
}
