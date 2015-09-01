#include "human.h"


double caldis(int x1,int x2,int y1,int y2)
{
	return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

CHuman::CHuman(uint32 index)
{
	m_index =index;
	frameindex = 0;
	frameNum = 0;

	m_colsZoomRate	= 2;
	m_rowsZoomRate	= 2;
	total = 0;

	alarm = 0;
	MaxNum = 0;
	m_Flag = 0;

	color = CV_RGB( 255, 0, 255 );
	color_rect = CV_RGB( 0, 255, 255);
	memset(&humanstatis, 0, sizeof(humanstatis));
	memset(&blobdata, 0 ,sizeof(blobdata));
	DirectionLines.clear();
	MonitorZoneRects.clear();
}

CHuman:: ~CHuman()
{
	DirectionLines.clear();
	MonitorZoneRects.clear();
	foregrondframe.release();
	mask.release();
}

void CHuman::sleep_release()
{
	alarm = 0;
	frameindex = 0;
	frameNum = 0;
}
void CHuman::pause_release()
{
	alarm = 0;
	frameindex = 0;
	frameNum = 0;
	memset(&humanstatis, 0, sizeof(humanstatis));
	memset(&blobdata, 0 ,sizeof(blobdata));
}

int CHuman::set_rectangle_line(vector<Rect> rect,vector<Line> line)
{

	uint8 i =0;
	MonitorZoneRects.clear();
	for(i = 0; i <rect.size(); i++)
	{
		Rect  tmp;
		tmp = rect[i];
		MonitorZoneRects.push_back(tmp);
	}

	uint8 j =0;
	DirectionLines.clear();
	for(j = 0; j<line.size(); j++)
	{
		Line  tmp;
		tmp = line[j];
		DirectionLines.push_back(tmp);
	}
	
	m_Flag = 0;
	if(DirectionLines.size() == 0 && MonitorZoneRects.size() == 0)
	{
		cout<<"camera "<<m_index<<" CHuman::VIDEO_DRAW DirectionLines and MonitorZoneRects size is 0"<<endl;
		return -1;
	}
	if(MonitorZoneRects.size()>0)m_Flag|=0x01;
	if(DirectionLines.size() > 0) m_Flag |= 0x02;
	return 0;
}
	
void  CHuman::doorwaydetect(int lineNum )
{
		int id = 0;
		//unsigned int total = 0;
		int flag = 0;
		int flag1 = 0;
		int flag2 = 0;
		int num = 0;
		int count = 0;
		if(humanlistpre.empty()) return;

		for(unsigned int i=0;i<humanlistpro.size();i++)//	 Frame.human.humanlist.size();i++)
		{
			id = humanlistpro[i].id;
			 flag1 = 1;
			 num = humanstatis.doorfilter[lineNum][id][0];
			 if(num == out)
			 {
				 for(int k=1;k<DOORFILTERLEVEAL/2;k++)
				 {
					 if(humanstatis.doorfilter[lineNum][id][k]!=num)
					 {
						 flag1 = 0;
						 break;
					 }

				 }
			 }
			 else// in on
			 {
				 for(int k=1;k<DOORFILTERLEVEAL/2;k++)
				 {
					 if(humanstatis.doorfilter[lineNum][id][k]==out)
					 {
						 flag1 = 0;
						 break;
					 }

				 }
			 }


			 flag2 = 1;
			 num = humanstatis.doorfilter[lineNum][id][DOORFILTERLEVEAL/2];
			 if(num == out)
			 {
				 for(int k=DOORFILTERLEVEAL/2 +1;k<DOORFILTERLEVEAL;k++)
				 {
					 if(humanstatis.doorfilter[lineNum][id][k]!=num)
					 {
						 flag2 = 0;
						 break;
					 }

				 }
			 }
			 else
			 {
				 for(int k=DOORFILTERLEVEAL/2 +1;k<DOORFILTERLEVEAL;k++)
				 {
					 if(humanstatis.doorfilter[lineNum][id][k]==out)
					 {
						 flag2 = 0;
						 break;
					 }

				 }
			 }

			 if(flag1&&flag2)
			 {
				 if((humanstatis.doorfilter[lineNum][id][0]==in)&&(humanstatis.doorfilter[lineNum][id][DOORFILTERLEVEAL/2 +1]<in))
				 {
						humanstatis.doorout[lineNum] ++;
						humanstatis.maxsize[lineNum]-=1;
						if(humanstatis.doorout[lineNum] > humanstatis.doorin[lineNum])
							humanstatis.doorout[lineNum] = humanstatis.doorin[lineNum];
				 }
				 else if((humanstatis.doorfilter[lineNum][id][0]==out)&&(humanstatis.doorfilter[lineNum][id][DOORFILTERLEVEAL/2 +1]>out))
				 {
						humanstatis.doorin[lineNum] ++;
				 }
			 }

		}

		if(humanstatis.maxsize[lineNum])
		{
			for(unsigned int i=0;i<DOORFILTERNUM;i++)//Frame.human.humanlistpro.size()
			{
				flag = 0;
				//for(unsigned int j=0;j<DOORFILTERLEVEAL1;j++)
				//{
					if(humanstatis.objdispear[lineNum][i] <25*4)
						flag = 1;
				//}

				if(!flag)//if(total == out)
				{

					humanstatis.doorout[lineNum] ++;
					if(humanstatis.maxsize[lineNum]>0)
						humanstatis.maxsize[lineNum]-=1;
					//for(unsigned int j=0;j<DOORFILTERLEVEAL1;j++)
					//{
					//	//Frame.humanstatis.dispear[lineNum][i][j] = in;
						humanstatis.objdispear[lineNum][i] = 0;
					//}
				}
			}
		}
	}


void  CHuman::blobdeal(Mat &displayframe)
{
	vector<dismin> blobdis;
	dismin vdis;
	dismin res;
	vector<dismin> dis;
	uint32  i=0;
	uint32 j =0;
	uint32 k =0;
	int id =0;
	int del_id = 0;
	int leftid = 0;
	int id_min = 0;
	int idmin = 0;
	int flag  = 0;

	//Frame.human.track  = Mat(Frame.variable.frame.rows, Frame.variable.frame.cols, CV_8UC3);
	//track  =displayframe.clone() ;  nikola
	humanlistpro = humanlist;

	if(humanlist.empty())
	{
		//printf("no objects\n");
		for(int lineNum =0;lineNum < DirectionLines.size();lineNum++)
		{
	//		humanstatis.jishu[lineNum] ++;
			for( i=0; i  < humanstatis.maxsize[lineNum];i++)
			{
				//cout<<"humanstatis.maxsize[lineNum]==="<<humanstatis.maxsize[lineNum]<<endl;
				//cout<<"humanstatis.objdispear[lineNum][i]==="<<humanstatis.objdispear[lineNum][i]<<endl;
					humanstatis.objdispear[lineNum][i]++;

			}

			if(humanstatis.maxsize[lineNum])
			{
				for(unsigned int i=0;i<DOORFILTERNUM;i++)//Frame.human.humanlistpro.size()
				{
					flag = 0;
					//for(unsigned int j=0;j<DOORFILTERLEVEAL1;j++)
					//{
						if(humanstatis.objdispear[lineNum][i] <25*4)
							flag = 1;
				/*	}*/

					if(!flag)//if(total == out)
					{

						humanstatis.doorout[lineNum] ++;
						humanstatis.maxsize[lineNum]-=1;
	/*					for(unsigned int j=0;j<DOORFILTERLEVEAL1;j++)
						{*/
							//Frame.humanstatis.dispear[lineNum][i][j] = in;
							humanstatis.objdispear[lineNum][i] = 0;
						/*}*/
					}
				}
			}
			/*
			if(humanstatis.jishu[lineNum]  >= 80)
			{
				humanstatis.doorout[lineNum] = humanstatis.doorin[lineNum];
				humanstatis.jishu[lineNum] = 100;
				humanstatis.maxsize[lineNum] = 0;
			}
*/
			for(int i=0;i<DOORFILTERNUM;i++)
				for(int j=0;j<DOORFILTERLEVEAL;j++)
					humanstatis.doorfilter[lineNum][i][j]=0;

		}
		return;
	}
	//memset(humanstatis.jishu,0,sizeof(humanstatis.jishu));

	if( humanlist.size() > humanlistpre.size())
	{
		for( i=0;i<humanlist.size();i++)
		{
			for(j=0;j<humanlistpre.size();j++)
			{

				vdis.dis=caldis(humanlist[i].x,humanlistpre[j].x,humanlist[i].y,humanlistpre[j].y);
				vdis.id = humanlistpre[j].id;
				blobdis.push_back(vdis);
			}

			if(!blobdis.empty ())
			{
				res = seekvectormin(blobdis);
				//Frame.human.humanlist[i].id = id;
				id = humanlistpro[i].id;
				humanlistpro[i].id = res.id;
				res.id = id;
				dis.push_back(res);
				blobdis.clear();
				//blobdis.resize(0);
			}
		}
		if(!dis.empty())
		{
			if(humanlist.size() == (humanlistpre.size() +1))
			{
				res = seekvectormax(dis);
				del_id = getid(dis,humanlistpre);
				humanlistpro[abs(res.id)].id = del_id;// 2 Frame.human.humanlist.size() - 1;
			}
			else if( humanlist.size()>=humanlistpre.size()+2)//have seperate
			{
				//int *index = new int[Frame.human.humanlistpre.size()];
				for( i=0;i<dis.size();i++)
				{
					for( j=0;j<humanlistpro.size();j++)
					{
						if(dis[i].id == humanlistpro[j].id)
						{
							//*index++ = dis[i].id;//
							//dis[i].id = 127;
							dis.erase(dis.begin() + i);
							break;
						}
					}
				}

				id_min = dis[0].id;
				for(i=0;i<dis.size();i++)
				{
					if(id_min >= dis[i].id)
					{
						id_min =dis[i].id;
						idmin  = i;
					}

				}

				for( k=0;k<dis.size();k++)
				{
					humanlistpro[dis[k].id].id = id_min;
				}
			}

			if(!dis.empty())  dis.clear();
		}
		//std::sort(dis.begin(),dis.end());
		//dis.back()
	}
	else//now < pre
	{
		for( i=0;i<humanlist.size();i++)
		{
			for( j=0;j<humanlistpre.size();j++)
			{
				vdis.dis=caldis(humanlist[i].x,humanlistpre[j].x,humanlist[i].y,humanlistpre[j].y);
				vdis.id = humanlistpre[j].id;
				blobdis.push_back(vdis);
			}

			if(!blobdis.empty())
			{
				res = seekvectormin(blobdis);
				humanlistpro[i].id = res.id;
				blobdis.clear();
				//blobdis.resize(0);
			}

		}

		for(int lineNum =0;lineNum < DirectionLines.size();lineNum++)
		{
			for( i=0;i<humanstatis.maxsize[lineNum];i++)//Frame.human.humanlistpre.size()
			{
				flag = 0;
				for(int j=0;j<humanlistpro.size();j++)
				{
					if(humanstatis.maxsize[lineNum] == humanlistpro[j].id)
						flag = 1;
				}
				if(!flag)
				{
				humanstatis.objdispear[lineNum][i]++;
				}
			}
		}
	}

	for(int lineNum =0;lineNum	<	DirectionLines.size();lineNum++)
	{
		for( i = 0;i<humanlistpro.size();i++)
		{
			id = humanlistpro[i].id;

			humanlistpro[i].pos[lineNum] = pointToline(DirectionLines[lineNum],Point2i(humanlistpro[i].x,humanlistpro[i].y));
			for(int j=0;j<DOORFILTERLEVEAL-1;j++)
				humanstatis.doorfilter[lineNum][id][j] = humanstatis.doorfilter[lineNum][id][j+1];
			humanstatis.doorfilter[lineNum][id][DOORFILTERLEVEAL-1] = humanlistpro[i].pos[lineNum];
			if(humanlistpro[i].pos[lineNum] == in)
			{
				//Frame.humanstatis.dispear[lineNum][id][DOORFILTERLEVEAL1-1] = in;
				humanstatis.objdispear[lineNum][i] = 0;
			}
		}

		doorwaydetect(lineNum);

	}


	humanlistpre = humanlistpro;

	for(int lineNum =0;lineNum < DirectionLines.size();lineNum++)
	{
		if(humanstatis.maxsize[lineNum]<humanstatis.doorin[lineNum] - humanstatis.doorout[lineNum])
				humanstatis.maxsize[lineNum] = humanstatis.doorin[lineNum] - humanstatis.doorout[lineNum];
	}

	for(unsigned int n=0;n<humanlistpro.size();n++)
	{
		Rect rt=Rect(humanlistpro[n].x-humanlistpro[n].w/2,humanlistpro[n].y-humanlistpro[n].h/2,humanlistpro[n].w,humanlistpro[n].h);
		rectangle(displayframe, rt, Scalar( 0, 255, 255 ), 2, 8, 0);
	}

}

void CHuman::human_detect(Mat &morph,Mat &displayframe)
{
	int centroidX = 0;
	int centroidY = 0;

	int label  = 0 ;
	int label1  =  0;

	for(unsigned int i = 0;i < m_BlobRects.size();i++)//while(pos)
	{

		Rect rt = m_BlobRects[i];
		if(rt.x + rt.width < morph.cols && rt.y + rt.height < morph.rows)
		{

			if(rt.width * rt.height<500)
				continue;

			centroidX = rt.x + rt.width/2;
			centroidY = rt.y + rt.height/2;

			blobdata.x = rt.x + rt.width/2;
			blobdata.y = rt.y + rt.height/2;
			blobdata.w = rt.width;
			blobdata.h = rt.height;
			blobdata.id = label1;
			Rect monitorzone =Rect(60, 0,displayframe.cols-60, displayframe.cols);
			if((centroidX > monitorzone.x)&&(centroidX < monitorzone.x +monitorzone.width)&&(centroidY>monitorzone.y)&&( centroidY < monitorzone.y +monitorzone.height) )
			{
				label1++;
				humanlist.push_back(blobdata);

			}

			blobdata.x = rt.x + rt.width/2;
			blobdata.y = rt.y + rt.height/2;
			blobdata.w = rt.width;
			blobdata.h = rt.height;
			blobdata.id = label;

			for(unsigned int k=0;k < MonitorZoneRects.size() ;k++)
			{
				if((centroidX > MonitorZoneRects[k].x)\
					&&(centroidX < MonitorZoneRects[k].x + MonitorZoneRects[k].width)\
					&&(centroidY > MonitorZoneRects[k].y)\
					&&(centroidY < MonitorZoneRects[k].y + MonitorZoneRects[k].height))
				{
					label++;
					object.push_back(cv::Point(centroidX,centroidY));
					//humanlist.push_back(blobdata);

					rectangle(displayframe, rt, Scalar( 255, 0, 0 ), 2, 8, 0);
					rectangle(displayframe,cvPoint(centroidX-1,centroidY-1),cvPoint(centroidX+1,centroidY+1),cvScalar(123,123,0),3,8,0);
					//alarm = 1;
					char idstr[100];
					sprintf(idstr,  "%d,%d",blobdata.id,blobdata.w*blobdata.h);
					putText(displayframe,idstr,cvPoint(blobdata.x,blobdata.y),CV_FONT_HERSHEY_COMPLEX, 1, cvScalar(0,0,255));
				}
			 }

		}
	}//for	end

}
void CHuman::algorithmMorphology_Operations( Mat& src, Mat& dst)
{
	if(0)
	{
		Mat element = getStructuringElement( cv::MORPH_RECT, Size( 12,12));
		Mat element1 = getStructuringElement( cv::MORPH_RECT, Size( 3,3 ));
		Mat tmp = Mat::zeros(src.rows, src.cols, src.type());
		//morphologyEx(src, tmp, MORPH_OPEN, element)
		//morphologyEx(tmp, dst, MORPH_CLOSE, element1);
		morphologyEx(src, tmp, MORPH_OPEN, element1);
		dilate(tmp, dst, element);
		//erode(dst, dst, element);
		tmp.release();
		element.release();
		element1.release();
	}
	else
	{
		Mat element = getStructuringElement( cv::MORPH_RECT, Size( 3,3));
		Mat element1 = getStructuringElement( cv::MORPH_RECT, Size( 3,3 ));
		Mat tmp = Mat::zeros(src.rows, src.cols, src.type());
		morphologyEx(src, tmp, MORPH_OPEN, element);
		morphologyEx(tmp, dst, MORPH_CLOSE, element1);
		//morphologyEx(src, tmp, MORPH_OPEN, element1)
		//dilate(tmp, dst, element);
		//erode(dst, dst, element);
		tmp.release();
		element.release();
		element1.release();
	}
}

void CHuman::census(Mat &displayframe)
{
	int humannum = 0;
	frameNum++;
/*
	for(unsigned int i=0; i<object.size(); i++)
	{
		//Point pt = object[i];
		if(object[i].x > (Frame.human.ThresholdX+Frame.human.ThresholdWidth))
		{
			humannum++;
		}
	}
*/
	humannum = humannum + object.size();
	humanstatis.numAll=humannum ;
	///*
	for(int i=0; i<NUM; i++)
	{
		humanstatis.statis[i] = humanstatis.statis[i+1];
		humanstatis.statis[NUM-1] = humannum;
	}
	if(frameNum == (NUM-1))
	{

        QuickSort(humanstatis.statis,0,NUM-1);
#if 0
		for(int i=2; i<NUM-2; i++)
			total += humanstatis[i];
		humannumAll = total/(NUM-4);
#endif
		humanstatis.numAll = humanstatis.statis[(NUM-1)/3*2];
		//cout<<"Frame.humanstatis.numAll==="<<Frame.humanstatis.numAll<<endl;
		frameNum = 0;
//*/
		if(humanstatis.numAll > humanstatis.prenum)
			humanstatis.inAll += humanstatis.numAll -humanstatis.prenum;
		else if(humanstatis.numAll < humanstatis.prenum)
			humanstatis.outAll += humanstatis.prenum - humanstatis.numAll;
			humanstatis.prenum = humanstatis.numAll;
	}
	//nikola
	//line(human.alarmCap,variable.varparam.DirectionLines[0].Start,variable.varparam.DirectionLines[0].End,Scalar(255));//Frame.human.alarmCap

}

T_HUMANNUM  CHuman::GetAlarmHumanNum()
{
	T_HUMANNUM t_HumanNum;
	memset(&t_HumanNum,0,sizeof(T_HUMANNUM));
	t_HumanNum.humanALL  	= humanstatis.numAll;
	t_HumanNum.humanIN		= MAX(humanstatis.doorin[0],humanstatis.doorin[1]);//human->humanstatis.inAll;
	t_HumanNum.humanOUT	  = MAX(humanstatis.doorout[0],humanstatis.doorout[1]);//human->humanstatis.outAll;
	for(int i=0; i<LINENUM;i++){
		t_HumanNum.doorIN[i]		= humanstatis.doorin[i];
		t_HumanNum.doorOUT[i]	  = humanstatis.doorout[i];
	}
	return t_HumanNum;
}

int CHuman::HumanDetectRun(Mat &displayframe)
{
	//int time_use=0;
	//struct timeval start;
 //struct timeval end;

  //gettimeofday(&start,NULL);

	Mat tmpframe;
	//Mat blobdealFrame;
	vector< vector<Point> >  contours;
	Rect contoursRect;

	alarm =0;

	displayframe.copyTo(tmpframe);
	//displayframe.copyTo(blobdealFrame);

	vector<blobnode>().swap(humanlistpro);

	 m_zoomRows  =   tmpframe.rows  /m_rowsZoomRate;
	 m_zoomCols  =   tmpframe.cols   /m_colsZoomRate;

	 w_Rate = (float)tmpframe.cols / m_zoomCols;
	 h_Rate = (float)tmpframe.rows / m_zoomRows;


	Mat morph = Mat(tmpframe.rows ,tmpframe.cols,CV_8UC1);

	mog(tmpframe,foregrondframe,0.001);   // 0.001

	frameindex++;
	if(frameindex<250) return 2;
	if(frameindex >= 250) frameindex =250;
	foregrondframe.copyTo(mask);
	threshold(mask, mask, 200, 255, THRESH_BINARY);

	cv::erode(mask, mask, cv::Mat());

	cv::dilate(mask, mask, cv::Mat());

	algorithmMorphology_Operations(mask, mask);

	findContours(mask, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

	mask.release(); //nikola
	m_BlobRects.clear();
	for(int i=0;i<contours.size();i++)
	{
		contoursRect = boundingRect(contours[i]);
		if(fabs(contourArea(contours[i])) > 600.0)
		{
			//rectangle(displayframe, contoursRect,color_rect, 2, 8, 0);
			m_BlobRects.push_back(contoursRect);
		}
	}

	if((m_Flag & 0x02)  == 0x02){
		for(int i=0;i<DirectionLines.size();i++)
		{
			line(displayframe,DirectionLines[i].Start,DirectionLines[i].End,Scalar(255));
		}

	}


	if((m_Flag & 0x01)  == 1){

		for(int ii=0;ii<MonitorZoneRects.size();ii++)
		{
			rectangle(displayframe, MonitorZoneRects[ii], Scalar( 255, 0, 0 ), 2, 8, 0);//��
		}
	}

	human_detect(morph,displayframe);

	if((m_Flag & 0x01)  == 1){
		census(displayframe);// for human statistics
	}

	if((m_Flag & 0x02)  == 0x02){
		blobdeal(displayframe);
	}

        if(humanstatis.numAll<(humanstatis.doorin[0]+humanstatis.doorin[1]-humanstatis.doorout[0]-humanstatis.doorout[1]))
        		humanstatis.numAll = humanstatis.doorin[0]+humanstatis.doorin[1]-humanstatis.doorout[0]-humanstatis.doorout[1];

	 //dbgprint("door1:in=%d,out=%d  door2:in=%d,out=%d\n",humanstatis.doorin[0],humanstatis.doorout[0],humanstatis.doorin[1],humanstatis.doorout[1]);

	if(humanstatis.numAll >= MaxNum){
				//printf("humanstatis.numAll is %d\n",humanstatis.numAll);
				alarm =1;
	}

	char dstr[100];
	sprintf(dstr, "door1:in=%d,out=%d  door2:in=%d,out=%d",humanstatis.doorin[0],humanstatis.doorout[0],humanstatis.doorin[1],humanstatis.doorout[1]);
	putText(displayframe,dstr,cvPoint(200,25),CV_FONT_HERSHEY_COMPLEX, 0.5, cvScalar(0,0,255));


	//printf("humanstatis Num:%d, humanstatisIn:%d,humanstatisOut:%d\n",humanstatis.numAll,humanstatis.inAll,humanstatis.outAll);


	//char dstr[100];
  //sprintf(dstr,  "in=%d,out=%d",humanstatis.doorin,humanstatis.doorout);
  //putText(displayframe,dstr,cvPoint(25,25),CV_FONT_HERSHEY_COMPLEX, 1, cvScalar(0,0,255));
	//printf("doorin=%d,doorout=%d\n",humanstatis.doorin,humanstatis.doorout);

	tmpframe.release(); //nikola

	morph.release();
	vector<Point>().swap(object); //vector<Point>
	vector<blobnode>().swap(humanlist);

	//gettimeofday(&end,NULL);
	//time_use=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000;//΢��
	//printf("time_use is %d\n",time_use);
	foregrondframe.release();

	return 0;
}
