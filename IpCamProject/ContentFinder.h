#pragma once

#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>

using namespace cv;

class ContentFinder
{
public:
	ContentFinder(void);
	~ContentFinder(void);
	void setThreshold(float th);
    float getThreshold() const;
    void setHistogram(const MatND &h);
    Mat find(const Mat &image, float minVal, float maxVal, int *chanenls, int dim);

private:
	float hranges[2];
    const float* ranges[3];
    int chanels[3];
    float threshold;
    MatND histogram;
};

