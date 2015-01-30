#pragma once
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <iostream>
#include <vector>
#define PATH_DATABASE "D:/Data/IpCam/database/ETSCameraClientCache.db3"

using namespace std;
using namespace cv;

class Enviroment
{
public:
	Enviroment(const cv::Mat& frame);
	~Enviroment(void);
	void drawEnviroment(cv::Mat& frame);
	void resizeFrame(cv::Mat& frame);

	//>>>Bốn điểm của vùng cửa hàng
	cv::Point TOP_LEFT;
	cv::Point TOP_RIGHT;
	cv::Point BOTTOM_LEFT;
	cv::Point BOTTOM_RIGHT;
	std::vector<cv::Point> POLY_POINT_OF_ROI;
	//<<<Bốn điểm của vùng cửa hàng

	//>>>Vùng cửa hàng
	int FRAME_WIDTH;
	int FRAME_HEIGHT;
	cv::Rect ROI;
	//<<<Vùng cửa hàng
};

