#pragma once

#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>

using namespace cv;

class ColorHistogram
{
public:
	ColorHistogram(void);
	~ColorHistogram(void);
    MatND getHistogram(const Mat &frame, Rect roi);
	void setThreshold(int th);
	int getThreshold();
private:
	int histSize[3]; // number bins
    float hranges[2]; //Min, max of value pixels
    const float* ranges[3];
    int chanels[3]; //numbers chanels;
	int threshold;
};

