// IpCamProject.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "IpCam.h"
#include "Tracker.h"
#include "Detector.h"
#include "Enviroment.h"
using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	/*IpCam ipCam;
	ipCam.run();*/


	Scalar Colors[]={
		Scalar(255,0,0),
		Scalar(0,255,0),
		Scalar(0,0,255),
		Scalar(255,255,0),
		Scalar(0,255,255),
		Scalar(255,0,255),
		Scalar(255,127,255),
		Scalar(127,0,255),
		Scalar(127,0,127)
	};

	VideoCapture capture("D:/Data/IpCam/Video/CLIP_20150115-140156.mp4");

	if(!capture.isOpened())
	{
		return 0;
	}
	namedWindow("Video");
	Mat frame;
	Mat gray;

	CTracker tracker(0.2, 0.5, 60.0, 10, 10);
	capture >> frame;

	//>>>Khởi tạo môi trường
	Enviroment enviroment(frame);
	enviroment.resizeFrame(frame);
	//<<<Khởi tạo môi trường

	cv::cvtColor (frame, gray,cv::COLOR_BGR2GRAY); 
	Detector* detector=new Detector(gray);
	//int k=0;
	vector<Point2d> centers;

	while(true)
	{
		capture >> frame;
		if(frame.empty())
		{
			capture.set(CV_CAP_PROP_POS_FRAMES,0);
			continue;
		}

		//>>> Resize
		enviroment.resizeFrame(frame);
		//<<< Resize

		cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

		centers = detector->Detect(gray);

		for(int i=0; i < centers.size(); i++)
		{
			circle(frame, centers[i], 3, Scalar(0,255,0), 1, CV_AA);
		}


		if(centers.size() > 0)
		{
			tracker.Update(centers, enviroment.ROI);

			cout << "Num of tracker:" << tracker.tracks.size()  << endl;

			for(int i=0;i<tracker.tracks.size();i++)
			{
				if(tracker.tracks[i]->trace.size()>1)
				{
					for(int j=0; j < tracker.tracks[i]->trace.size()-1; j++)
					{
						line(frame,tracker.tracks[i]->trace[j],tracker.tracks[i]->trace[j+1],Colors[tracker.tracks[i]->track_id%9],2,CV_AA);
					}
				}
			}
		}

		//>>> Vẽ vùng cửa hàng
		enviroment.drawEnviroment(frame);
		//<<< Vẽ vùng cửa hàng

		imshow("Video",frame);

		if(waitKey(20)> 0)
		{
			break;
		}
	}

	capture.release();
	delete detector;
	destroyAllWindows();
	return 0;
}

