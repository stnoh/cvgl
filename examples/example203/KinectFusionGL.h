/******************************************************************************
New API to utilize KinectFusion routine in GL viewer
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#ifndef KINECT_FUSION_GL
#define KINECT_FUSION_GL

#include <NuiKinectFusionApi.h>


///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
class KinectFusionGL
{
public:
	KinectFusionGL();
	~KinectFusionGL();

	// override member functions
	bool Init();
	void End();
};

#endif
