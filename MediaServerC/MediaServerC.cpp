// MediaServerC.cpp : ��������̨Ӧ�ó��������ڵ㡣
//

#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "AnalyzeCamera.h"
#include "MediaServerDLL.h"


CAnalyzeCamera AnalyzeCamera;

void *ThreadFun(void *arg)
{
	FILE  *pF = fopen("40s.h264", "rb");
	if(pF == NULL)return 0;

	unsigned char *pBUF = new unsigned char[1500];
	while( 1 )
	{
		int nret = fread(pBUF,1,1360,pF);

		if( nret > 0 )
		{
			if( AnalyzeCamera.m_capture_callback != NULL )
				AnalyzeCamera.m_capture_callback(0, nret, pBUF, 600300001);
		}
		else{
			break;
		}

		usleep(40*1000);
	}
	delete[] pBUF;


	pthread_t Media1Thread;

	int retRTSPServer = StartRTSPServer(&AnalyzeCamera);

	pthread_create(&Media1Thread,NULL,ThreadFun,NULL);
	pthread_detach(Media1Thread);


	return NULL;
}

int main(int argc, char* argv[])
{
	//CNULLCamera NULLCamera;
	//int retRTSPServer = StartRTSPServer(&NULLCamera);

	//CAnalyzeCamera AnalyzeCamera;

	int iRet = -1;
	pthread_t MediaThread;

	int retRTSPServer = StartRTSPServer(&AnalyzeCamera);

	iRet = pthread_create(&MediaThread,NULL,ThreadFun,NULL);

	pthread_detach(MediaThread);


	while(true)sleep(300);
	StopRTSPServer();

	return 0;
}
