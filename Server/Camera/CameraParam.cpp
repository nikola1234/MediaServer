#include "CameraParam.h"
#include "FileOpr.h"

extern T_ServerParam SerParam;

CamParam::CamParam()
{
  CameraID = 0;
}

CamParam::~CamParam()
{

}

int CamParam::get_camera_rtspurl(string &rtspurl)
{
  char rtsp[64] ={0};
  uint32 port =558;
  if(strlen(SerParam.Serverip) == 0) return -1;
  sprintf(rtsp,"%s%s:%d/%d","rtsp://",SerParam.Serverip,port,CameraID);
  rtspurl = rtsp;
  return  0;
}
