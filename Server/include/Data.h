#ifndef _DATA_H_
#define _DATA_H_

#include "Common.h"
#include "CmdDefine.h"

typedef struct _CAM_LIST{
	uint32 CameraID;
	char url[SINGLE_URL_LEN_128];
	  _CAM_LIST(){
		memset(this, 0, sizeof(_CAM_LIST));
	}
}T_CAM_LIST;

#endif
