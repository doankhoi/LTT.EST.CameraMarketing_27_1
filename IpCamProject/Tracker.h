#pragma once
#include <iostream>
#include <vector>
#include "Kalman.h"
#include "HungarianAlg.h"
#include <time.h>
#include "ConnectDB.h"
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;


class CTrack
{
public:
	vector<Point2d> trace; //Các điểm theo vết
	static size_t NextTrackID;
	size_t track_id;
	size_t skipped_frames; //Số khung hình mất dấu

	Point2d prediction; //Vị trí dự đoán
	TKalmanFilter* KF;
	time_t timestart; //Thời gian bắt đầu theo vết
	bool is_calc_time; //Biến dùng để kiểm tra có tính thời gian hay không

	CTrack(Point2f p, float dt, float Accel_noise_mag);
	~CTrack();
};

class CTracker
{
public:
	//>>>Kết nối database
	sqlite3* db;
	ConnectDB connectDb;
	string SHOP_CD;
	//<<<Kết nối database

	float dt; 

	float Accel_noise_mag;

	double dist_thres; //Đặt ngưỡng cho khoảng cách có thể chấp nhận được
	int maximum_allowed_skipped_frames; //Ngưỡng cho phép số khung hình mất vết
	
	int max_trace_length;
	vector<CTrack*> tracks; //Lưu các đối tượng chuyển động
	vector<CTrack*> tracks_static;//Lưu các đối tượng đứng yên tại khung hình trước

	bool isIn(cv::Point2d center, const cv::Rect& roi);//Kiểm tra nằm trong cửa hàng
	void Update(vector<Point2d>& detections, const cv::Rect& roi);//Cập nhật danh sách theo vết

	CTracker(float _dt, float _Accel_noise_mag, double _dist_thres=60, int _maximum_allowed_skipped_frames=10, int _max_trace_length=10);
	~CTracker(void);
};

