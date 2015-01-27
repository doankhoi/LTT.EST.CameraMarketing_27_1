#include "stdafx.h"
#include "ColorHistogram.h"


ColorHistogram::ColorHistogram(void)
{
	histSize[0] = histSize[1] = histSize[2] = 255;
	hranges[0] = 0.0f;
	hranges[1] = 255.0f;

	ranges[0] = hranges;
	ranges[1] = hranges;
	ranges[2] = hranges;

	chanels[0] = 0;
	chanels[1] = 1;
	chanels[2] = 2;
	threshold = 65;
}

ColorHistogram::~ColorHistogram(void)
{
}

void ColorHistogram::setThreshold(int th)
{
	this->threshold = th;
}

int ColorHistogram::getThreshold()
{
	return this->threshold;
}

MatND ColorHistogram::getHistogram(const Mat& frame, Rect roi){
    MatND hist;
	Mat imgROI = frame(roi);
    //Chuyen doi RBG sang HUE
    Mat hsv;   
    cvtColor(imgROI, hsv, CV_RGB2HSV);
    
    //Kiem tra minSat cua anh
    Mat mask;
	if(threshold > 0){
        vector<Mat> v;
        split(hsv, v); //Tach ra 3 anh       
		cv::threshold(v[1], mask, threshold, 255, CV_THRESH_BINARY);
    }
    
    hranges[0] = 0.0f;
    hranges[1] = 180.0f;
    chanels[0]=0;
    
    calcHist(&hsv, 1, chanels, mask, hist, 1, histSize, ranges);
    
    return hist;
}
