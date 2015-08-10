#include "interface.h"
#include <iostream>

using namespace std;


void onTrackbarSlide( int, void* )
{

}
//øÏÀŸ≈≈–Ú
///////////////////////////////////////
void Swap(int &a,int &b)
{
    int temp;
    temp=a;
    a=b;
    b=temp;
}

int Partition(int a[],int p,int r)
{
    int i=p;
    int j=r+1;
    int x=a[p];
    while (true)
    {
        while(a[++i]<x&&i<r);
        while(a[--j]>x);
        if (i>=j)break;
        Swap(a[j],a[i]);

    }
    a[p]=a[j];
    a[j]=x;
    return j;
}

void QuickSort(int a[],int p,int r)
{
    if (p<r)
    {
        int q=Partition(a,p,r);
        QuickSort(a,p,q-1);
        QuickSort(a,q+1,r);
    }
}

int seekmax(int *arry, int k)
{
	int max = 0;
	max = *(arry + 0);

	for(int i = 1; i < k; i++)
	{
		if (*(arry + i) > max)
			max = *(arry + i);
	}
    return max;
}

int seekmin(int *arry, int k)
{
	int min = 0;
	min = *(arry + 0);

	for(int i = 1; i < k; i++)
	{
		if (*(arry + i) < min)
			min = *(arry + i);
	}
    return min;
}
#if 0
int seekvectormin(vector<dismin> *arry)
{
	int min = 0;
	int id = 0;
	int num = (*arry).size();
	min = (*arry)[0].dis;//*((*arry) + 0);

	for(int i = 1; i < num; i++)
	{
		if ((*arry)[i].dis < min)
		{
			min = (*arry)[i].dis;
			id  = (*arry)[i].id;
		}
	}
    return id;
}
#endif

#if 1
dismin seekvectormin(vector<dismin> &arry)
{
	int min = 0;
	int id = 0;
	int num = arry.size();
	min = arry[0].dis;//*((*arry) + 0);
	int pt = 0;
	for(int i = 1; i < num; i++)
	{
		if (arry[i].dis < min)
		{
			min = arry[i].dis;
			id  = arry[i].id;
			pt  = i;
		}
	}
    return arry[pt];
}
#endif
dismin seekvectormax(vector<dismin> &arry)
{
	int max = 0;
	int id = 0;
	int num = arry.size();
	max = arry[0].dis;//*((*arry) + 0);
	int pt = 0;
	for(int i = 1; i < num; i++)
	{
		if (arry[i].dis > max)
		{
			max = arry[i].dis;
			id  = arry[i].id;
			pt  = i;
		}
	}
    return arry[pt];
}
#if 0
int getid(vector<dismin> &a,vector<blobnode> &b)
{
	vector<dismin> *r_id;
	r_id = &a;
	int id = 0;
	for(int i=0;i<b.size();i++)
	{
		id = b[i].id;
		for(int j=0;j<a.size();j++)
		{
			if(id == a[j].id)
			{
				r_id->erase(r_id->begin() + j);
			}
		}
	}
	cout<<"YYYYYYY.size=="<<r_id->size()<<endl;
	return seekvectormin(*r_id).id;
}
#endif
int getid(vector<dismin> &a,vector<blobnode> &b)
{
	int id = 0;
	for(int i=0;i<b.size();i++)
	{
		id = b[i].id;
		for(int j=0;j<a.size();j++)
		{
			if(id == a[j].id)
			{
				a.erase(a.begin() + j);
				break;
			}
		}
	}
	return seekvectormin(a).id;
}

bool pointTorect(CvPoint &point,Rect &rect)
{
	if((point.x>rect.x)&&(point.x<rect.x+rect.width)&&(point.y>rect.y)&&(point.y<rect.y+rect.height))
		return in;
	else 
		return out;
}

position pointToline(Line &line,Point2i point)
{
	int kx = 0;
	int ky = 0;
	if(line.Start.x == line.End.x)
	{
		if(point.x > line.Start.x)
			return in;
		else if(point.x < line.Start.x)
			return out;
		else
			return on;
	}
	else if(line.Start.y == line.End.y)
	{
		if(point.y > line.Start.y)
			return in;
		else if(point.y < line.Start.y)
			return out;
		else
			return on;
	}
	else
	{
		if(line.End.x<line.Start.x)
		{
			swap(line.End.x,line.Start.x);
			swap(line.End.y,line.Start.y);
		}
		kx =line.End.x - line.Start.x;
		ky =line.End.y - line.Start.y;
		if((kx*point.y - ky*point.x + ky*line.Start.x - kx*line.Start.y) > 0)
			return in;
		else if((kx*point.y - ky*point.x + ky*line.Start.x - kx*line.Start.y) < 0)
			return out;
		else
			return on;
	}
}
#if 0
	uchar *b_data=NULL;
    	uchar *prev_img=NULL;
    	uchar *current_img=NULL;
	prev_img = prev_image.ptr<uchar>(0);
	current_img = image.ptr<uchar>(0);
	if(i == 1){
	i=0;
	update_bg(240,160,prev_img,current_img);
	}

		for(int j=0;j<c_size;j++){
                for(int k=0;k<p_size;k++){
                        if(abs(kp_last[k].x-kp[j].x) < (kp[j].w) && abs(kp_last[k].y-kp[j].y) < (kp[j].h))
                        { 
				 meet++;
				 kp[j].id = id_count;
                                 kp_last[k].id = id_count;	
                	}	
		}
                if(meet >= 2) {
			for(int k=0;k<c_size;k++){
                                if(kp[k].id == id_count)
                                        kp[k].num = meet;
                        }
		//	cout << meet << "     persons.....meet............" << endl;
		}
		id_count++;
                meet = 0;
        }
		/*
void putText(Mat &img,char*   text,int x,int y)  
{  
    CvFont  font;  
    double  hscale  =   1.0;  
    double  vscale  =   1.0;  
    int linewidth   =   2;  
    cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC,hscale,vscale,0,linewidth);  
    CvScalar    textColor   =cvScalar(255,0,0);  
    CvPoint textPos =cvPoint(x, y);  
    cvPutText((CvMat*)&img,  text,   textPos,    &font,textColor);  
} 
*/
//void putText( Mat& img, const string& text, Point org,int fontFace, double fontScale, Scalar color,int thickness=1, int lineType=8, bool bottomLeftOrigin=false );
#endif