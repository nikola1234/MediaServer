#ifndef _HUMAN_H_
#define _HUMAN_H_

#include "Common.h"

typedef struct _LINE_{
	Point2i Start;
	Point2i End;
}Line;

#define LINENUM          2

typedef enum {
	normal,
	meet,
	separate,
	newadd
}en_state;

typedef enum {
	out=0,
	in=1,
	on
}position;

typedef struct _dismin_
{
	char id;
	long dis;

}dismin;

typedef struct _blobnode_
{
	int x;
	int y;
	int w;
	int h;
	char id;
	dismin dis;
	en_state state;
	position pos[LINENUM];
	//unsigned int pos[LINENUM];
	int obs_size;
	int contact;

} blobnode;

//#define OLD_ALGO
//#define TRAIN_MODE
//#define DEBUG_OUTPUT

#ifdef TRAIN_MODE
extern bool trainComplete;
#define NUM              20
#define DOORFILTERLEVEAL 12
#define DOORFILTERLEVEAL1 12000
#define DOORFILTERNUM    20



typedef struct _statistics_
{
	int totalnumMax;
	int inAll;
	int outAll ;
	int numAll ;
	int prenum ;
	int statis[NUM];
	int doorin[LINENUM];
	int doorout[LINENUM];
	int doorfilter[LINENUM][DOORFILTERNUM][DOORFILTERLEVEAL];
//	int dispear[LINENUM][DOORFILTERNUM][DOORFILTERLEVEAL1];
	int objdispear[LINENUM][DOORFILTERNUM];
	int maxsize[LINENUM];
	int  jishu[LINENUM];

	//vector< int > doorin;
	//vector< int > doorout;
	//vector< vector< int > > doordetecsta;
}statistics;

class CHuman
{
public:
	CHuman(uint8 index);
	~CHuman();

	uint8    	alarm;
	uint8 	m_index;
	int     m_rowsZoomRate;
	int	    m_colsZoomRate;

	int     m_zoomRows;
	int     m_zoomCols;

	float 	w_Rate;
	float		h_Rate;

	CvScalar color ;
	CvScalar color_rect ;

	uint32 frameindex;
	uint16 frameNum;

	cv::BackgroundSubtractorMOG2 mog;
	Mat foregrondframe;
	Mat mask;
	vector< cv::Rect >			m_BlobRects;

	int total ;
	vector<Point> object;
	vector< blobnode >          humanlist;
	vector< blobnode >          humanlistpre;
	vector< blobnode >          humanlistpro;
	blobnode blobdata;

	Mat track;

	statistics   humanstatis;
	void  doorwaydetect(int lineNum);
	void  blobdeal(Mat &displayframe);
	void  human_detect(Mat &morph,Mat &displayframe);
	void algorithmMorphology_Operations( Mat& src, Mat& dst);
	void census(Mat &displayframe);
	int HumanAlarmRun(Mat &displayframe);

};

#endif
