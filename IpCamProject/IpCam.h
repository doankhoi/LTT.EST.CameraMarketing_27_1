#pragma once

#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

//Include opencv
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\video\video.hpp>
#include <opencv2\video\background_segm.hpp>
#include <opencv2\video\tracking.hpp>

#include <math.h>
#include <stdio.h>
//C++
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <time.h>
#include "ConnectDB.h"
#include "Utils.h"
#include "Blob.h"
#include "MeanShiftTracker.h"
#include "MeanShift.h"
#include "ColorHistogram.h"
#include "watershedsegmenter.h"

using namespace std;
using namespace cv;

class IpCam
{
public:
	IpCam(void);
	~IpCam(void);

	//bool sortByFrameCount(const Blob &lhs, const Blob &rhs);

	//Hàm chuyển đổi int sang string
	string intToString(int number);

	//Hàm vẽ đường danh giới
	void drawLine(cv::Mat &frame, cv::Point a, cv::Point b);

	//Hàm vẽ vùng ROI
	void drawROI(cv::Mat &frame, vector<Point> v);

	//Hàm kiểm tra gặp nhau của hai khung hình
	bool checkCollisionRect(cv::Rect rect1, cv::Rect rect2);

	//Theo vết đối tượng với Camshift
	void trackCamshift(cv::Mat &image, cv::Rect initWindow, int vmin, int vmax, int smin, cv::Rect &rectOutput);

	//Tiền sử lý ảnh
	void preProcess(Ptr<BackgroundSubtractor> mog, const Mat& frame, Mat &output);
	
	//Tìm khung hình bằng matchTemplate voi nhieu khung hinh
	void findMatchTemplateMulti(Mat &referFrame, Blob &blob);

	//Tim khung hinh bang matchTemplat voi khung hinh don
	void findMatchTemplateSingle(Mat &referFrame, Blob &blob);

	void run();

private:
	VideoCapture cap;
	ConnectDB connectDb;
	WatershedSegmenter segmenter;

	sqlite3 *db;
	int threshold;

	//Các tỉ lệ
	int MIN_SCALE;
	int MAX_SCALE;

	float SCALE_HEIGHT;
	float SCALE_WIDTH;
	int FRAME_WIDTH;
	int FRAME_HEIGHT;

	cv::Point BOTTOM_LEFT;
	cv::Point BOTTOM_RIGHT;
	cv::Point TOP_LEFT;
	cv::Point TOP_RIGHT;
};

