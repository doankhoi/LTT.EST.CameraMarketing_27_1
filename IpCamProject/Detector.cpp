#include "stdafx.h"
#include "Detector.h"


Detector::Detector(Mat& gray)
{
	fg = gray.clone();
	bs = new BackgroundSubtract;
	bs->init(gray);
	vector<Rect> rects;
	vector<Point2d> centers;
}


Detector::~Detector(void)
{
	delete bs;
}

void Detector::DetectContour(Mat& img, vector<Rect>& Rects,vector<Point2d>& centers)
{
	double area=0;
	Rects.clear();
	centers.clear();

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	Mat edges = img.clone();

	//>>>Tách đối tượng
	Mat dist;
	imshow("edges", edges);
	cv::distanceTransform(edges, dist, CV_DIST_L2, 3);
	cv::normalize(dist, dist, 0, 1.0, cv::NORM_MINMAX);
	cv::threshold(dist, dist, 0.5, 1.0, CV_THRESH_BINARY);
	cv::Mat dist_8u;
	imshow("dist", dist);
	dist.convertTo(dist_8u, CV_8U);

	//Canny(img, dist_8u, 50, 190, 3);
	findContours(dist_8u,contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point());
	//<<<Tách đối tượng

	if( contours.size() > 0)
	{
		for( int i = 0; i < contours.size(); i++ )
		{
			Rect r=cv::boundingRect(contours[i]);

			//Nếu đối tượng quá nhỏ bỏ qua
			if(r.area() > MIN_AREA_PERSON)
			{
				Rects.push_back(r);
				centers.push_back((r.br()+r.tl())*0.5);
			}
		}
	}
}


vector<Point2d> Detector::Detect(Mat& gray)
{
	bs->subtract(gray, fg);
	DetectContour(fg, rects, centers);

	return centers;
}
