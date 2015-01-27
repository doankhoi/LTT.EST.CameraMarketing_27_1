#include "stdafx.h"
#include "ContentFinder.h"


ContentFinder::ContentFinder(void):threshold(-1.0f)
{
	ranges[0] = hranges;
	ranges[1] = hranges;
	ranges[2] = hranges;
}


ContentFinder::~ContentFinder(void)
{
}

void ContentFinder::setThreshold(float th) 
{
	this->threshold = th;
}

float ContentFinder::getThreshold() const 
{
	return this->threshold;
}

void ContentFinder::setHistogram(const MatND& h) 
{
	histogram = h;
	normalize(histogram, histogram, 1.0);
}

Mat ContentFinder::find(const Mat& image, float minVal, float maxVal, int* chanenls, int dim) 
{

	Mat result;
	hranges[0] = minVal;
	hranges[1] = maxVal;

	for (int i = 0; i < dim; i++) 
	{
		this->chanels[i] = chanenls[i];
	}

	calcBackProject(&image, 1, chanenls, histogram, result, ranges, 255);

	//Loại bỏ các màu đặc trưng yếu
	if (threshold > 0.0) {
		cv::threshold(result, result, 255*threshold, 255, CV_THRESH_BINARY);
	}

	return result;
}
