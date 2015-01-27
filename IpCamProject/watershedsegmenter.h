#pragma once
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\video\background_segm.hpp>
#include <opencv2\video\video.hpp>
#include <opencv2\video\tracking.hpp>
#include <opencv2\contrib\contrib.hpp>
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

class WatershedSegmenter
{
public:
	WatershedSegmenter(){}
	~WatershedSegmenter(){}

	void setMarkers(const cv::Mat& markerImage) {
		// Convert to image of ints
		markerImage.convertTo(markers,CV_32S);
	}

	cv::Mat process(const cv::Mat &image) {
		// Apply watershed
		cv::watershed(image,markers);
		markers.convertTo(markers, CV_8U);
		return markers;
	}

private:
	cv::Mat markers;
};
