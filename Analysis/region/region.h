#ifndef _REGION_H_
#define _REGION_H_

#include "Common.h"

class CRegion
{

public:
	CRegion(uint8 index);
	~CRegion();

	uint8 m_index;
	uint8 alarm;

	vector< Rect > Rects;
	uint16 frameindex;
	int SetRectangle(vector< Rect > & rectangle);
	int RegionDetectRun(Mat &dispalyFrame);

private:
	Mat TmpFrame;


	float resolution;

	int     m_rowsZoomRate;
	int	    m_colsZoomRate;

	int     m_zoomRows;
	int     m_zoomCols;

	float 	w_Rate;    /* width and height rate */
	float		h_Rate;

	Mat mask;

	cv::BackgroundSubtractorMOG2 mog;

	Mat foregrondframe;

	void motiondetective(Mat &dispalyFrame,Mat &morph);
	void algorithmMorphology_Operations( Mat& src, Mat& dst );
};

#endif
