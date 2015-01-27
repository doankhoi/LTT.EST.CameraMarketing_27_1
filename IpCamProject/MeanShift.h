#pragma once
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\video\background_segm.hpp>
#include <opencv2\video\tracking.hpp>
#include <opencv2\video\video.hpp>
#include <vector>
#include "ContentFinder.h"
using namespace cv;

class MeanShift
{
public:
	MeanShift(void);
	~MeanShift(void);
	cv::Rect process(const cv::Mat &frame, cv::MatND hist, int threshold);
	void setTargetCurrent(cv::Rect rect);
	cv::Rect getTargetCurrent();
private:
	ContentFinder finder;
	Rect target_current; //Khung hinh theo vet hien tai
};

