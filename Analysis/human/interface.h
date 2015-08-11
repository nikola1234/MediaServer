#pragma once

#include "Common.h"

#define LINENUM          2

typedef struct _LINE_{
	Point2i Start;
	Point2i End;
}Line;

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

void onTrackbarSlide( int, void* );

void Swap(int &a,int &b);
int Partition(int a[],int p,int r);
void QuickSort(int a[],int p,int r);

int seekmax(int *arry, int k);
int seekmin(int *arry, int k);
//int seekvectormin(vector<dismin> *arry);
dismin seekvectormin(vector<dismin> &arry);
dismin seekvectormax(vector<dismin> &arry);
int getid(vector<dismin> &a,vector<blobnode> &b);
//int getid(vector<dismin> *a,vector<blobnode> &b);
bool pointTorect(CvPoint &point,Rect &rect);
position pointToline(Line &line,Point2i point);
