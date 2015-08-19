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


typedef struct _ALARM_TIME{

    uint8 hour;
    uint8 min;

}T_AlarmTime;  /* time structure h-m*/

typedef struct _ALARM_TIME_INT{

    T_AlarmTime Start;
    T_AlarmTime End;

}ALARM_TIME_INT;

typedef struct _ALARM_DAY_INT{

    ALARM_TIME_INT time[3];

}ALARM_DAY_INT;

typedef struct _ALARM_DAY{

	uint8 En;
 	ALARM_DAY_INT dayTime;

}ALARM_DAY;

#endif
