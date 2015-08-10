//
//  VideoHandler.cpp
//  FlameDetection
//
//  Created by liberize on 14-4-6.
//  Copyright (c) 2014年 liberize. All rights reserved.
//

#include "VideoHandler.h"

VideoHandler::VideoHandler(int device, bool saveKeyFrame, bool saveVideo)
: mCapture(device)
, mSaveKeyFrame(saveKeyFrame)
, mSaveVideo(saveVideo)
, mFromCam(true)
, mVideoFPS(0)
{
    if (mCapture.isOpened()) {
        mVideoFPS = mCapture.get(CV_CAP_PROP_FPS);
        if (mVideoFPS == 0) {
            mVideoFPS = 8.0;
        }
    }
}

VideoHandler::VideoHandler(const string& file, bool saveKeyFrame)
: mCapture(file)
, mSaveKeyFrame(saveKeyFrame)
, mFromCam(false)
, mVideoFPS(0)
{
    if (mCapture.isOpened()) {
        mVideoFPS = mCapture.get(CV_CAP_PROP_FPS);
        assert(mVideoFPS != 0);
    }
}
VideoHandler::VideoHandler()
{
}

int VideoHandler::handle(Mat &displayFrame , Rect ROI,void* videoHandler)
{

        displayFrame.copyTo(mFrame);

	if( (0 <= ROI.x && 0 <= ROI.width && ROI.x + ROI.width <= mFrame.cols && 0 <= ROI.y && 0 <= ROI.height && ROI.y + ROI.height <= mFrame.rows) ){

	        mFrame = mFrame(ROI);
	       
		if(mDetector.detect(mFrame,videoHandler))  {
			return STATUS_FLAME_DETECTED;
		}
		return STATUS_NO_FLAME_DETECTED;
	}else{
		printf("ROI.x = %d ,ROI.y = %d ,ROI.width =%d ,ROI.height",ROI.x ,ROI.y,ROI.width ,ROI.height);
		dbgprint("fire alarm wrong ROI !\n");
		return STATUS_NO_FLAME_DETECTED;
	}
	
}

bool VideoHandler::saveFrame()
{
    string fileName;
    getCurTime(fileName);
    fileName += ".jpg";
    cout << "Saving key frame to '" << fileName << "'." << endl;

    return imwrite(fileName, mFrame);
}

bool VideoHandler::saveVideo()
{
    if (mSaveVideoFile.empty()) {
        getCurTime(mSaveVideoFile);
        mSaveVideoFile += ".mov";
        cout << "Saving video to '" << mSaveVideoFile << "'." << endl;
        
        // in Mac OS X, only 'mp4v' is supported
        int fourcc = CV_FOURCC('m', 'p', '4', 'v');
        Size size = Size((int)mCapture.get(CV_CAP_PROP_FRAME_WIDTH),
                         (int)mCapture.get(CV_CAP_PROP_FRAME_HEIGHT));

        mWriter.open(mSaveVideoFile, fourcc, mVideoFPS, size, true);
    }

    if (!mWriter.isOpened()) {
        return false;
    }

    mWriter << mFrame;
    return true;
}
