#include "stdafx.h"
#include "Enviroment.h"

Enviroment::Enviroment(const cv::Mat& frame)
{
	//Giới hạn khung hình
	POLY_POINT_OF_ROI.clear();

	float SCALE_HEIGHT = 0.7;
	float SCALE_WIDTH = 0.2;

	FRAME_WIDTH = (int)(frame.cols)/3;
	FRAME_HEIGHT = (int)(frame.rows)/3;

	//Bốn điểm danh giới
	BOTTOM_LEFT = cv::Point( static_cast<int>(FRAME_WIDTH* SCALE_WIDTH)- 30, static_cast<int> (FRAME_HEIGHT * SCALE_HEIGHT));

	BOTTOM_RIGHT = cv::Point(FRAME_WIDTH - static_cast<int>(static_cast<int>(FRAME_WIDTH* SCALE_WIDTH)), static_cast<int> (FRAME_HEIGHT * SCALE_HEIGHT));

	TOP_LEFT = cv::Point(static_cast<int>(FRAME_WIDTH* SCALE_WIDTH)-30,static_cast<int>(FRAME_HEIGHT * 0.05));
	TOP_RIGHT = cv::Point(FRAME_WIDTH - static_cast<int>(static_cast<int>(FRAME_WIDTH* SCALE_WIDTH)), static_cast<int>(FRAME_HEIGHT * 0.05));

	POLY_POINT_OF_ROI.push_back(BOTTOM_LEFT);
	POLY_POINT_OF_ROI.push_back(BOTTOM_RIGHT);
	POLY_POINT_OF_ROI.push_back(TOP_RIGHT);
	POLY_POINT_OF_ROI.push_back(TOP_LEFT);

	//Vùng ảnh theo vết trong cửa hàng
	ROI = cv::Rect(TOP_LEFT.x, TOP_LEFT.y, TOP_RIGHT.x - TOP_LEFT.x, BOTTOM_LEFT.y - TOP_LEFT.y);
}


Enviroment::~Enviroment(void)
{
}

void Enviroment::resizeFrame(cv::Mat& frame)
{
	cv::resize(frame, frame, cv::Size(FRAME_WIDTH, FRAME_HEIGHT));
}

void Enviroment::drawEnviroment(cv::Mat& frame)
{
	
	int size = POLY_POINT_OF_ROI.size();
	for(int i=1; i< size; i++){
		cv::line(frame, POLY_POINT_OF_ROI[i-1], POLY_POINT_OF_ROI[i], cv::Scalar(255, 0, 0), 2);
	}

	cv::line(frame, POLY_POINT_OF_ROI[0], POLY_POINT_OF_ROI[size-1], cv::Scalar(255, 0, 0), 2);
}

