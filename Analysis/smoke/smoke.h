#ifndef _SOMKE_H_
#define _SOMKE_H_

#include "Common.h"

class CSmoke
{
public:
	CSmoke(uint8 index);

	uint8 m_index;
	uint8  alarm;
	bool alarmIs;

	float waveletThres;
	bool initialized;
	vector<vector<Point> > blobs;
	vector<vector<Point> > lastblobs;
	vector<Rect> SmokeRegion;
	Mat element;
	int blockSize;
	Mat frame1;
	Mat frame2;
	Mat frame3;
	Mat background;
	Mat foreground;
	Mat curRefinedFg;
	Mat curMorph_Foreground;
	Mat prevMorph_Foreground;
	Mat prevFrame;

	Mat EMap[3];

	Mat thres;
	Mat	bgEMap;

	void initThreshold(int cols, int rows, float initValue) ;
	void updateBackground(Mat& frame1, Mat& frame2, Mat& frame3);
	void SmokeDetectorMorphology_Operations( Mat& src, Mat& dst);
	void SmokeDetectorlabelBlobs(const cv::Mat &binary);
	void getEdgeModel(Mat& frame) ;
	Rect getBoundaryofBlob(vector<Point> blob);
	float getBinaryMotionMask(Mat& prevFrame,Mat& curFrame, Mat& motionMask, vector<Point> blob) ;
	float getWeberContrast(Mat& inputFrame, Mat& background, vector<Point> blob) ;
	void smokeDetect(Mat& frame1, Mat& frame2, Mat& frame3) ;
	vector<Rect>  detectSmoke(Mat& originFrame) ;
	int SmokeAlarmDetectRun(Mat & displayframe);

};

#endif
