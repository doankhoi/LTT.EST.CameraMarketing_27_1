#pragma once
#include "OpencvInclude.h"
#include "VIBE.h"

class BackgroundSubtract
{
public:
	BackgroundSubtract(void);
	~BackgroundSubtract(void);
	void init(cv::Mat &image);
	void subtract(const cv::Mat &image, cv::Mat &foreground);
private:
	vibeModel_t *model;
};

