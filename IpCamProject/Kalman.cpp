#include "stdafx.h"
#include "Kalman.h"


#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
using namespace cv;
using namespace std;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TKalmanFilter::TKalmanFilter(Point2f pt,float dt,float Accel_noise_mag)
{
	//Thời gian dùng cho ma trận chuyển dịch
	deltatime = dt; //0.2

	//Khởi tạo các ma trận chuyển vị, hiệp phương sai
	//4 state variables, 2 measurements
	kalman = new KalmanFilter( 4, 2, 0 );  
	// Transition matrix
	kalman->transitionMatrix = (Mat_<float>(4, 4) << 1,0,deltatime,0,   0,1,0,deltatime,  0,0,1,0,  0,0,0,1);

	// init... 
	LastResult = pt;
	kalman->statePre.at<float>(0) = pt.x; // x
	kalman->statePre.at<float>(1) = pt.y; // y

	kalman->statePre.at<float>(2) = 0;
	kalman->statePre.at<float>(3) = 0;

	kalman->statePost.at<float>(0)= pt.x;
	kalman->statePost.at<float>(1)= pt.y;

	setIdentity(kalman->measurementMatrix);

	kalman->processNoiseCov=( Mat_<float>(4, 4) << 
		pow(deltatime,4.0)/4.0      ,0	                    ,pow(deltatime,3.0)/2.0, 0,	
		0                           ,pow(deltatime,4.0)/4.0	,0			           , pow(deltatime,3.0)/2.0,
		pow(deltatime,3.0)/2.0	    ,0						,pow(deltatime,2.0)	   , 0,
		0						    ,pow(deltatime,3.0)/2.0	,0					   ,pow(deltatime,2.0));


	kalman->processNoiseCov *= Accel_noise_mag;

	setIdentity(kalman->measurementNoiseCov, Scalar::all(0.1));

	setIdentity(kalman->errorCovPost, Scalar::all(.1));

}

//---------------------------------------------------------------------------
TKalmanFilter::~TKalmanFilter()
{
	delete kalman;
}

//---------------------------------------------------------------------------
Point2f TKalmanFilter::GetPrediction()
{
	Mat prediction = kalman->predict();
	LastResult = Point2f(prediction.at<float>(0),prediction.at<float>(1)); 
	return LastResult;
}

//---------------------------------------------------------------------------
Point2f TKalmanFilter::Update(Point2f p, bool DataCorrect)
{
	Mat measurement(2,1,CV_32FC1);

	if(!DataCorrect)
	{
		measurement.at<float>(0) = LastResult.x;  //cập nhật theo prediction
		measurement.at<float>(1) = LastResult.y;
	}
	else
	{
		measurement.at<float>(0) = p.x;  //cập nhật theo measurements
		measurement.at<float>(1) = p.y;
	}
	// Correction
	Mat estimated = kalman->correct(measurement);
	LastResult.x=estimated.at<float>(0);   //cập nhật theo measurements
	LastResult.y=estimated.at<float>(1);

	return LastResult;
}
//---------------------------------------------------------------------------