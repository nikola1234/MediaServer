
#include "CameraManage.h"

ManageCamera::ManageCamera()
{

}

ManageCamera::~ManageCamera()
{

}

int ManageCamera::InitFromDB()
{
  	return 0;
}

int ManageCamera::try_to_open(string stream)
{
	VideoCapture  vcap;
	 if(!vcap.open(stream)) {
	        return -1;
	}
	return 0;
}
