#pragma once

#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\video\video.hpp>
#include <opencv2\video\background_segm.hpp>
#include <opencv2\video\tracking.hpp>
#include "MeanShiftTracker.h"
#include <vector>

using namespace cv;
using namespace std;

class Blob
{
public:
	Blob(void);
	~Blob(void);
	int ID;
	int firstFrameNumber;
	int lastFrameNumber;

	int frameCount;
	time_t startTime;
	cv::Rect lastRectangle;
	cv::Rect prevRectangle;
	cv::Rect matchRectangle;

	//MeanShiftTracker meanShiftTracker;
	cv::MatND hist_current;
	//std::vector<int> contactContours;
	//cv::vector<cv::Mat> frames; //Danh sách các vùng ảnh ROI qua các frame
	cv::vector<cv::Rect> rectsTrack;
	cv::Mat frameROI;
};

