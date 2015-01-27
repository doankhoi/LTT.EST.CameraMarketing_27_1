#include "stdafx.h"
#include "Utils.h"


Utils::Utils(void)
{
}


Utils::~Utils(void)
{
}

void Utils::eraseContours(vector<vector<Point> > &contours, int minPoint, int maxPoint)
{
	vector<vector<Point> >::iterator it = contours.begin();
	while (it != contours.end()) 
	{
		if ((it->size() < minPoint) || (it->size() > maxPoint)) {
			it = contours.erase(it);
		} else {
			it++;
		}
	}
}

void Utils::eraseContours(vector<vector<Point> > &contours, cv::Rect roi)
{
	vector<vector<Point> >::iterator it = contours.begin();
	cv:: Rect rect;
	while (it != contours.end()) 
	{
		rect = cv::boundingRect(*it);

		if (this->isIn(rect, roi)) 
		{
			it = contours.erase(it);
		} else {
			it++;
		}
	}
}

void Utils::eraseBlob(vector<Blob> &blobs, int numMaxFrame)
{
	vector<Blob>::iterator it = blobs.begin();

	while(it != blobs.end())
	{
		if(it->frameCount >= numMaxFrame)
		{
			it = blobs.erase(it);
		}else{
			it++;
		}
	}
}

bool Utils::checkRect(cv::Rect rect, int minValue, int maxValue)
{
	if ((rect.width > minValue) && (rect.width < maxValue)) 
	{
		return true;
	}

	return false;
}

int Utils::getNumBlob(Mat foregroundFrameBuffer, Rect roi, int minValue, int maxValue)
{
	int num = 0;
	Mat roiFrame = foregroundFrameBuffer(roi).clone();
	vector<vector<Point> > contours;

	findContours(roiFrame, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	int size = contours.size();

	for(int i=0; i< size; i++){
		Rect rect = boundingRect(contours[i]);
		if(checkRect(rect, minValue, maxValue))
		{
			num++;
		}
	}

	return num;
}

Blob Utils::getMaxRect(vector<Blob> blobs)
{
	int size = blobs.size();
	int index = 0;
	int maxArea = 0;
	Rect rect;
	for(int i=0; i< size; i++)
	{
		rect = blobs[i].lastRectangle;
		if((rect.area()) > maxArea)
		{
			index = i;
			maxArea = rect.area();
		}
	}
	
	return blobs[index];
}


bool Utils::isGoOut(cv::Rect prevRect, cv::Rect lastRect, cv::Rect roi)
{
	//Tính tâm của hai khung hình trước và sau
	int prevX = prevRect.x + prevRect.width/2;
	int prevY = prevRect.y + prevRect.height/2;
	int lastX = lastRect.x + lastRect.width/2;
	int lastY = lastRect.y + lastRect.height/2;

	//Tính tọa độ bốn điểm của vùng cửa hàng.
	int topLeftX = roi.x;
	int topLeftY = roi.y;
	int topRightX = roi.x + roi.width;
	int bottomLeftY = roi.y + roi.height;

	if( (prevX > topLeftX)&& 
		(lastX > topLeftX)&&
		(prevX < topRightX)&&
		(lastX < topRightX)&&
		(prevY > topLeftY)&& 
		(lastY > topLeftY)&&
		(prevY < bottomLeftY)&&
		(lastY < bottomLeftY)
		)
	{
		return false;
	}

	return true;
}

bool Utils::isGoIn(cv::Rect prevRect, cv::Rect lastRect, cv::Rect roi)
{
	//Tính tâm của hai khung hình trước và sau
	int prevX = prevRect.x + prevRect.width/2;
	int prevY = prevRect.y + prevRect.height/2;

	int lastX = lastRect.x + lastRect.width/2;
	int lastY = lastRect.y + lastRect.height/2;

	//Tính tọa độ bốn điểm của vùng cửa hàng.
	int topLeftX = roi.x;
	int topLeftY = roi.y;

	int topRightX = roi.x + roi.width;
	int bottomLeftY = roi.y + roi.height;

	if((prevX <= topLeftX)&&(lastX > topLeftX)&&(prevY <= bottomLeftY) && (lastY <= bottomLeftY))
	{
		return true;
	}

	if((prevX >=topRightX)&&(lastX < topRightX) && (prevY <= bottomLeftY) && (lastY <= bottomLeftY))
	{
		return true;
	}

	if( (prevX >= topLeftX)&&
		(prevX <= topRightX)&&
		(lastX >= topLeftX)&&
		(lastX < topRightX)&&
		(prevY >= bottomLeftY)&&
		(lastY < bottomLeftY))
	{ 
		return true;
	}
	return false;
}

bool Utils::deleteElement(vector<Blob> &blobs, int index)
{
	vector<Blob>::iterator it = blobs.begin();
	int i = 0;
	while (it != blobs.end()) 
	{
		if (index == i) {
			it = blobs.erase(it);
			return true;
		} else {
			it++;
			i++;
		}
	}

	return false;
}

void Utils::drawToRect(cv::Mat &frame, Rect prev, Rect curr, cv::Scalar scalar)
{
	//Tính tọa độ y của tâm hai khung hình
	int aX = prev.x + prev.width/2;
	int aY = prev.y + prev.height / 2;

	int bY = curr.y + curr.height / 2;
	int bX = curr.x + curr.width/2;

	cv::Point point1(aX, aY);
	cv::Point point2(bX, bY);

	cv::line(frame, point1, point2, scalar, 1, 8);
}

void Utils::drawTrack(cv::Mat &frame, Blob blob, cv::Scalar scalar) 
{
	vector<Rect> rects = blob.rectsTrack;
	int size = rects.size();

	for(int i=1; i< size ;i++)
	{
		drawToRect(frame, rects[i-1], rects[i], scalar);
	}
}

vector<vector<Point> > Utils::findContoursOfRoi(cv::Mat frame, cv::Rect roi)
{

	Mat frameRoi = frame(roi).clone();
	blur(frameRoi, frameRoi, Size(4, 4));

	Ptr<BackgroundSubtractorMOG2> ptr = new BackgroundSubtractorMOG2(300, 32, true);
	Mat foreImage, binary;
	ptr->operator()(frameRoi, foreImage, -1.0);
	ptr->set("nmixtures", 3);

	//Giảm nhiễu
	Mat element(5, 5, CV_8U, cv::Scalar(1));
	morphologyEx(foreImage, binary, MORPH_CLOSE, element);
	threshold(binary, binary, 128, 255, CV_THRESH_BINARY);

	vector<vector<Point> > contours;

	findContours(binary, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	eraseContours(contours, 50, 10000);

	return contours;
}

bool Utils::isIn(cv::Rect rect, cv::Rect roi)
{
	int x = rect.x + rect.width/2;
	int y = rect.y + rect.height/2;

	int roiTopLeftX = roi.x;
	int roiTopLeftY = roi.y;
	int roiBottomLeftY = roi.y + roi.height;
	int roiTopRightX = roi.x +  roi.width;

	if((x >=roiTopLeftX)&&(x <= roiTopRightX)&&(y >= roiTopLeftY)&&(y <= roiBottomLeftY))
	{
		return true;
	}

	return false;
}

void Utils::addPeopleContainerTrack(Mat frame, vector<Blob> &containerTrack, cv::Mat foreFrame, cv::Rect roi, int minValue, int maxValue, int &ID, int frameNumber)
{
	
	vector<vector<Point>> contours;
	cv::findContours(foreFrame, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	int size = contours.size();

	cv::Rect rect;

	for(int i=0; i< size; i++)
	{
		rect = cv::boundingRect(contours[i]);

		//Nếu đối tượng trong cửa hàng và có kích thước vượt ngưỡng cho phép
		if((this->isIn(rect, roi)) &&(this->checkRect(rect, minValue, maxValue)))
		{
			Mat roiFrameBuffer = frame(rect).clone();

			Blob blob;
			blob.ID = ID++;
			blob.frameCount = 1;
			blob.prevRectangle = rect;
			blob.lastRectangle = rect;
			blob.firstFrameNumber = frameNumber;
			blob.lastFrameNumber = frameNumber;

			blob.rectsTrack.push_back(rect);
			time_t current;
			time(&current);
			blob.startTime = current;

			containerTrack.push_back(blob);
			//cout << "Them mot doi tuong k" << endl;
		}
	}
}

double Utils::getDistance(cv::Rect r1, cv::Rect r2)
{
	int x1 = r1.x + r1.width/2;
	int y1 = r1.y + r1.height/2;
	int x2 = r2.x + r2.width/2;
	int y2 = r2.y + r2.height/2;

	double dis = sqrt(pow(x1-x2,2)+ pow(y1-y2, 2));
	return dis;
}

bool Utils::isImpact(Rect rect1, Rect rect2)
{
	
	if((std::max(rect1.x, rect2.x) < std::min(rect1.x+rect1.width, rect2.y+rect2.width))&&
		(std::max(rect1.y, rect2.y) < std::min(rect1.y+rect1.height, rect2.y+rect2.height)))
       return true;
	return false;
}