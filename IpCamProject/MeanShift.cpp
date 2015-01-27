#include "stdafx.h"
#include "MeanShift.h"

MeanShift::MeanShift(void)
{
}


MeanShift::~MeanShift(void)
{
}

void MeanShift::setTargetCurrent(cv::Rect rect)
{
	this->target_current = rect;
}

cv::Rect MeanShift::getTargetCurrent()
{
	return this->target_current;
}

cv::Rect MeanShift::process(const cv::Mat &frame, cv::MatND hist, int threshold)
{
	finder.setHistogram(hist);

	cv::Mat hsv;
	cvtColor(frame, hsv, CV_BGR2HSV);
	vector<Mat> v;

	split(hsv, v);
	cv::threshold(v[1], v[1], threshold, 255, cv::THRESH_BINARY);
	cv::Mat result;
	int ch[3];
	ch[0]=0;
	ch[1]=1;
	ch[2]=2;
	result = finder.find(hsv, 0.0f, 180.0f, ch, 1);
	cv::bitwise_and(result, v[1], result);

	Rect fRect = getTargetCurrent();
	TermCriteria criteria(cv::TermCriteria::MAX_ITER, 10, 0.01);
	cv::meanShift(result, fRect, criteria);

	return fRect;
}


