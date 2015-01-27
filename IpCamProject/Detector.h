#pragma once

#include <opencv2\opencv.hpp>
#include "BackgroundSubtract.h"
#include <iostream>
#include <vector>
#define MIN_AREA_PERSON 60

using namespace cv;
using namespace std;

class Detector
{
public:
	Detector(Mat& gray);
	~Detector(void);
	vector<Point2d> Detect(Mat& gray);

private:
	void DetectContour(Mat& img, vector<Rect>& Rects,vector<Point2d>& centers);
	BackgroundSubtract* bs;
	vector<Rect> rects;
	vector<Point2d> centers;
	Mat fg;
};

