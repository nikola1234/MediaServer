#pragma once

#include "Common.h"

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

