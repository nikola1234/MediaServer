#ifndef _SINGLE_CAMERA_H_
#define _SINGLE_CAMERA_H_

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "Common.h"
#include "CameraAnalyze.h"
#include "CameraParam.h"

class SingleCamera
:public boost::enable_shared_from_this<SingleCamera>
{
public:
	SingleCamera();
	~SingleCamera();
	
		
private:
	uint32 CameraID;
	CamParam * Param;
	CamAnalyze *Ana;
};
#endif