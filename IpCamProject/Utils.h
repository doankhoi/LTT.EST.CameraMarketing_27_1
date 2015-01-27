#pragma once
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\video\video.hpp>
#include <opencv2\video\background_segm.hpp>
#include <opencv2\video\tracking.hpp>
#include <vector>
#include <string>
#include <time.h>
#include <ctime>
#include <iostream>
#include "Blob.h"

#define MY_THRESHOLD  40

using namespace cv;
using namespace std;

class Utils
{
public:
	Utils(void);
	~Utils(void);

	//Tính số blob trong vùng ảnh roi
	int getNumBlob(Mat foregroundFrameBuffer, Rect roi, int minValue, int maxValue);

	//Bổ sung danh sách người theo vết
	void addPeopleContainerTrack(Mat frame, vector<Blob> &containerTrack, cv::Mat foreFrame, cv::Rect roi, int minValue, int maxValue, int &ID, int frameNumber);

	//Xóa các khung hình quá bé hoặc quá lớn không nên tính
	void eraseContours(vector<vector<Point> > &contours, int minPoint, int maxPoint);

	//Loại bỏ các khung hình nằm trong cửa hàng
	void eraseContours(vector<vector<Point> > &contours, cv::Rect roi);

	//Loại bỏ các blob quá lâu không xét tới
	void eraseBlob(vector<Blob> &blobs, int numMaxFrame);

	//Hàm kiểm tra Rect nếu max chiều rộng hoặc chiều cao lớn hơn minValue, nhỏ maxValue 
	bool checkRect(cv::Rect rect, int minValue, int maxValue);

	//Tìm khung hình lớn nhất
	Blob getMaxRect(vector<Blob> blobs);

	//Hàm kiểm tra đối tượng đi ra cửa hàng không
	bool isGoOut(cv::Rect prevRect, cv::Rect lastRect, cv::Rect roi);

	//Hàm kiểm tra đối tượng đi vào cửa hàng hay không
	bool isGoIn(cv::Rect prevRect, cv::Rect lastRect, cv::Rect roi);

	//Xóa một phần tử có index trong vector
	bool deleteElement(vector<Blob> &blobs, int index);

	//Vẽ đường nối tâm của hai hình chữ nhật
	void drawToRect(cv::Mat &frame, Rect prev, Rect curr, cv::Scalar scalar);

	//Vẽ đường theo vết một Blob
	void drawTrack(cv::Mat &frame, Blob blob, cv::Scalar scalar);

	//Tìm khung hình của vùng roi
	vector<vector<Point> > findContoursOfRoi(cv::Mat frame, cv::Rect roi);

	//Hàm kiểm tra một khung hình có nằm trong vùng cửa hàng không
	bool isIn(cv::Rect rect, cv::Rect roi);

	//Lay khoang cach giua hai hinh
	double getDistance(cv::Rect r1, cv::Rect r2);

	//Kiem tra va cham cua hai Rect
	bool isImpact(Rect rect1, Rect rect2);
};

