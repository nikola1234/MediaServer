#pragma once

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"

using namespace std;
using namespace cv;

#define NONE 0  // no filter
#define HARD 1  // hard shrinkage
#define SOFT 2  // soft shrinkage
#define GARROT 3  // garrot filter
float sgn(float x);
float soft_shrink(float d,float T);
float hard_shrink(float d,float T);
float Garrot_shrink(float d,float T);
void cvHaarWavelet(Mat &src,Mat &dst,int NIter);
void cvInvHaarWavelet(Mat &src,Mat &dst,int NIter, int SHRINKAGE_TYPE, float SHRINKAGE_T);
void getEnergyMap(Mat& frame, Mat& ret);
void getFilteredFrame(Mat& frame, Mat& filtered);