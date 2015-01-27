#include "stdafx.h"
#include "IpCam.h"

#define MIN_TIMEIN 10.0

IpCam::IpCam(void): threshold(128)
{
	(this->db) = (this->connectDb).connectDb("C:\\Program Files\\EVSoft\\ETS Shop Client\\ETSCameraClientCache.db3");	
}


IpCam::~IpCam(void)
{
}

bool sortByFrameCount(const Blob &lhs, const Blob &rhs)
{
	return lhs.frameCount > rhs.frameCount;
}


string IpCam::intToString(int number) 
{
	stringstream ss;
	ss << number;
	return ss.str();
}

void IpCam::drawLine(cv::Mat &frame, cv::Point a, cv::Point b) 
{
	cv::line(frame, a, b, cv::Scalar(255, 0, 0), 2);
}

void IpCam::drawROI(cv::Mat &frame, vector<Point> v)
{


	int size = v.size();
	for(int i=1; i< size; i++){
		this->drawLine(frame, v[i-1], v[i]);
	}

	this->drawLine(frame, v[0], v[size-1]);
}

bool IpCam::checkCollisionRect(cv::Rect rect1, cv::Rect rect2)
{
	if ((std::max(rect1.x, rect2.x) <= std::min(rect1.x + rect1.width, rect2.x + rect2.width))&&
		(std::max(rect1.y, rect2.y) <= std::min(rect1.y + rect1.height, rect2.y + rect2.height))) {
			return true;
	} 
	return false;
}

void IpCam::trackCamshift(cv::Mat &image, cv::Rect initWindow, int vmin, int vmax, int smin, cv::Rect &rectOutput)
{
	cv::Rect trackWindow = initWindow;
	Mat hsv, hue, hist, histimg, mask, backproj;
	cv::cvtColor(image, hsv, COLOR_BGR2HSV);
	cv::inRange(hsv, Scalar(0, smin, MIN(vmin,vmax)), Scalar(180, 256, MAX(vmin, vmax)), mask);
	int ch[] = {0, 0};

	hue.create(hsv.size(), hsv.depth());
	mixChannels(&hsv, 1, &hue, 1, ch, 1);

	Mat roi(hue, trackWindow), maskroi(mask, trackWindow);
	int hsize = 16;
	float hranges[] = {0,180};
	const float* phranges = hranges;

	calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
	normalize(hist, hist, 0, 255, CV_MINMAX);
	histimg =  Mat::zeros(200, 320, CV_8UC3);

	histimg = Scalar::all(0);
	int binW = histimg.cols / hsize;

	Mat buf(1, hsize, CV_8UC3);

	for(int i = 0; i < hsize; i++ )
		buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);

	cvtColor(buf, buf, CV_HSV2BGR);

	/*for( int i = 0; i < hsize; i++ )
	{
	int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows/255);
	rectangle( histimg, Point(i*binW,histimg.rows), Point((i+1)*binW,histimg.rows - val), Scalar(buf.at<Vec3b>(i)), -1, 8 );
	}*/

	calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
	backproj &= mask;
	RotatedRect trackBox = CamShift(backproj, trackWindow, TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));

	if( trackWindow.area() <= 1 )
	{
		int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;

		trackWindow = Rect(trackWindow.x - r, trackWindow.y - r, trackWindow.x + r, trackWindow.y + r) &
			Rect(0, 0, cols, rows);
	}

	rectOutput = trackWindow;
}

void IpCam::preProcess(Ptr<BackgroundSubtractor> mog, const Mat& frame, Mat &output)
{
	Mat foreFrame, binaryImg, frameBlur;
	Mat element = getStructuringElement(MORPH_RECT, Size(7, 7), Point(3, 3));
	//Blur
	blur(frame, frameBlur, Size(4, 4));
	//Binary
	mog->operator()(frameBlur, foreFrame, -1);

	//pre procesing
	morphologyEx(foreFrame, binaryImg, cv::MORPH_CLOSE, element);
	cv::threshold(binaryImg, binaryImg, this->threshold, 255, CV_THRESH_BINARY);

	Mat fg, bg, result;
	cv::erode(binaryImg, fg, cv::Mat(), cv::Point(-1,-1), 6);
	cv::dilate(binaryImg, bg, cv::Mat(), cv::Point(-1,-1),6);
	cv::threshold(bg, bg, 1, 128, CV_THRESH_BINARY_INV);
	cv::Mat markers(binaryImg.size(), CV_8U, cv::Scalar(0));
	markers = fg+bg;
	segmenter.setMarkers(markers);
	result = segmenter.process(frameBlur);
	cv::threshold(result, result, 128, 255, CV_THRESH_BINARY);
	output = result;
}

void IpCam::findMatchTemplateMulti(Mat &referFrame, Blob &blob)
{
	Mat temp = blob.frameROI;
	cv::Rect rect_prev = blob.prevRectangle;

	if (referFrame.empty() || temp.empty())
		return;

	cv::Mat gref, gtpl;
	cv::cvtColor(referFrame, gref, CV_BGR2GRAY);
	cv::cvtColor(temp, gtpl, CV_BGR2GRAY);

	cv::Mat res(referFrame.rows - temp.rows+1, referFrame.cols - temp.cols+1, CV_32FC1);
	cv::matchTemplate(gref, gtpl, res, CV_TM_CCOEFF_NORMED);
	cv::threshold(res, res, 0.8, 1., CV_THRESH_TOZERO);

	//Lay tat ca cac khung hinh co the la doi tuong tim cai gan nhat
	int k = 0;
	double minDist, dis;
	Utils utils;
	cv::Rect rect_result(0, 0, 0, 0);

	while (true) 
	{
		double minval, maxval, threshold = 0.8;
		cv::Point minloc, maxloc;
		cv::minMaxLoc(res, &minval, &maxval, &minloc, &maxloc);

		if (maxval >= threshold)
		{
			//Láº¥y khung hÃ¬nh tÃ¬m kiáº¿m
			Rect rect(maxloc.x, maxloc.y, temp.cols, temp.rows);
			dis = utils.getDistance(rect, rect_prev);

			if(k==0)
			{
				minDist = dis;
				rect_result = rect;
				k++;
				continue;
			}

			if(dis < minDist)
			{
				minDist = dis;
				rect_result = rect;
			}

		}
		else
			break;
	}//end while

	//Cáº­p nháº­t láº¡i 
	if(rect_result.width==0 || rect_result.height == 0)
		return;

	Mat v = referFrame(rect_result);
	blob.frameROI = v;
	blob.matchRectangle = rect_result;

	//Ve Id cua doi tuong
	cv::putText(referFrame, "( "+this->intToString(blob.ID)+ ")", cv::Point(rect_result.x, rect_result.y), FONT_HERSHEY_PLAIN, 2, Scalar(0,255,0),1);

	cv::rectangle(referFrame, rect_result, cv::Scalar(0, 255, 0), 1);
}

void IpCam::findMatchTemplateSingle(Mat &referFrame, Blob &blob)
{
	cv::Mat temp = blob.frameROI;
	if(referFrame.empty() || temp.empty())
	{
		cout  << "Not match" << endl;
		return;
	}

	cv::Mat gref, gtpl;
	cv::cvtColor(referFrame, gref, CV_BGR2GRAY);
	cv::cvtColor(temp, gtpl, CV_BGR2GRAY);

	cv::Mat result(referFrame.rows - temp.rows + 1, referFrame.cols - temp.cols + 1, CV_32FC1);

	cv::matchTemplate(gref, gtpl, result, CV_TM_CCOEFF_NORMED);
	cv::normalize(result, result, 0, 1, NORM_MINMAX, -1, cv::Mat());

	double minVal, maxVal;
	cv::Point minLoc, maxLoc;

	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());

	cv::Rect matchRect(maxLoc.x, maxLoc.y, temp.cols, temp.rows);
	//Ve id cua doi tuong
	//cv::putText(referFrame, "( "+this->intToString(blob.ID)+ ")", cv::Point(matchRect.x, matchRect.y), FONT_HERSHEY_PLAIN, 2, Scalar(0,255,0),1);
	//cv::rectangle(referFrame, matchRect, cv::Scalar(0, 255, 0), 1);

	//cap nhat lai khung hinh
	cv::Mat v;
	v = referFrame(matchRect);
	blob.matchRectangle = matchRect;
	blob.frameROI = v;
}

string SHOP_CD = "";

void IpCam::run()
{
	string url = (this->connectDb).getCameraInfo(this->db);
	if(url =="")
	{
		cout << "カメラ情報を設定してください。"<< endl;
		return;
	}
	SHOP_CD = (this->connectDb).getShopInfo(this->db);
	if(SHOP_CD =="")
	{
		cout << "店舗コードを設定してください。"<< endl;
		return;
	}

	url = "D:/EstellseCamera/Camera Video/CLIP_20150115-133158.mp4";

	//Khai báo
	Mat foregroundFrameBuffer, contoursFrame, roiFrameMask, roiFrame, roiFrameBuffer, rawFrame, rawCopyFrame, foregroundFrame;

	Ptr<BackgroundSubtractor> mog;
	MeanShift meanShiftTracker;
	ColorHistogram colorHistogram;

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	cv::TermCriteria criteria(cv::TermCriteria::MAX_ITER, 100, 1.0);

	(this->cap).open(url);

	if(!(this->cap).isOpened())
	{
		cout << "Error: Not connect to: " << url << endl;
		return;
	}

	//Danh sách các ứng cử viên
	vector<Blob> blobContainer;
	vector<Blob>::iterator itBlob;

	Rect bRect;

	//Danh sách các Blob người được vẽ trong vòng lặp trước
	vector<Blob> peopleContainerTrack;
	vector<Blob>::iterator itPeople;

	vector<Blob>::iterator it;
	vector<Blob>::iterator itBlobContainer;

	//----end khai báo
	mog = new BackgroundSubtractorMOG2(300, 32, true);//(300, 32, true)
	mog->set("nmixtures", 3);

	int frameNumber = 0;
	int numPeopleContainerTrack = 0;
	int numBlobOfRoi = 0;
	Utils utils;
	int ID = 0;
	time_t current;
	time(&current);
	int index = (this->connectDb).getNewstIdCustomer(this->db, SHOP_CD, current);

	//Giới hạn khung hình
	vector<Point> polyPointOfROI;
	SCALE_HEIGHT = 0.65;
	SCALE_WIDTH = 0.2;

	FRAME_WIDTH = cap.get(CV_CAP_PROP_FRAME_WIDTH)/3;
	FRAME_HEIGHT = cap.get(CV_CAP_PROP_FRAME_HEIGHT)/3;

	MIN_SCALE = std::max(FRAME_WIDTH, FRAME_HEIGHT) / 50;
	MAX_SCALE = std::max(FRAME_WIDTH, FRAME_HEIGHT) / 2;

	//Bốn điểm danh giới

	BOTTOM_LEFT = cv::Point( static_cast<int>(FRAME_WIDTH* SCALE_WIDTH)- 30, static_cast<int> (FRAME_HEIGHT * SCALE_HEIGHT));
	BOTTOM_RIGHT = cv::Point(FRAME_WIDTH - static_cast<int>(static_cast<int>(FRAME_WIDTH* SCALE_WIDTH)), static_cast<int> (FRAME_HEIGHT * SCALE_HEIGHT));

	TOP_LEFT = cv::Point(static_cast<int>(FRAME_WIDTH* SCALE_WIDTH)-30,static_cast<int>(FRAME_HEIGHT * 0.05));
	TOP_RIGHT = cv::Point(FRAME_WIDTH - static_cast<int>(static_cast<int>(FRAME_WIDTH* SCALE_WIDTH)), static_cast<int>(FRAME_HEIGHT * 0.05));

	polyPointOfROI.push_back(BOTTOM_LEFT);
	polyPointOfROI.push_back(BOTTOM_RIGHT);
	polyPointOfROI.push_back(TOP_RIGHT);
	polyPointOfROI.push_back(TOP_LEFT);

	//Vùng ảnh theo vết trong cửa hàng
	cv::Rect roi(TOP_LEFT.x, TOP_LEFT.y, TOP_RIGHT.x - TOP_LEFT.x, BOTTOM_LEFT.y - TOP_LEFT.y);

	//Vùng ảnh ngoai cửa hàng
	cv::Rect roiDetect( 0, BOTTOM_LEFT.y, FRAME_WIDTH, FRAME_HEIGHT - BOTTOM_LEFT.y);

	//----end lấy vùng ảnh quan trọng ---

	//Tạo trackbar
	cv::namedWindow("Frame",1);
	cv::createTrackbar("Threshold", "Frame", &(this->threshold), 255);

	while (true) {

		if (!(cap.read(rawFrame))) 
			break;

		frameNumber++;

		if (rawFrame.empty()) {
			std::cout << "Finish" << endl;
			return;
		}

		
		//Chuyển đổi kích thước
		resize(rawFrame, rawFrame, Size(FRAME_WIDTH, FRAME_HEIGHT));
		rawFrame.copyTo(rawCopyFrame);

		drawROI(rawFrame, polyPointOfROI);

		//Tiền xử lý ảnh
		preProcess(mog, rawFrame, foregroundFrameBuffer);

		imshow("fore", foregroundFrameBuffer);

		//-----Đếm lại các đối tượng trong vùng cửa hàng -------
		numBlobOfRoi = utils.getNumBlob(foregroundFrameBuffer, roi, MIN_SCALE, MAX_SCALE);
		//std::cout << "NumBlobOfRoi:" << numBlobOfRoi << endl;

		//-------End đếm lại các đối tượng trong vùng cửa hàng.

		//Phát hiện thêm đối tượng
		cv::findContours(foregroundFrameBuffer, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		//Dự đoán khung hình 
		itPeople = peopleContainerTrack.begin();
		while(itPeople != peopleContainerTrack.end())
		{
			//Dung MeanShift du doan dich chuyen.
			Rect rect = itPeople->lastRectangle;
			meanShiftTracker.setTargetCurrent(rect);
			Rect fRect = meanShiftTracker.process(rawCopyFrame, itPeople->hist_current, MY_THRESHOLD);

			//Kiem tra khoang cach giua fRect va lastRectangle
			if(utils.isImpact(fRect, itPeople->lastRectangle))
			{
				//Hoan vi
				itPeople->frameROI = rawFrame(itPeople->lastRectangle);
				itPeople->prevRectangle = itPeople->lastRectangle;
				itPeople->lastRectangle = fRect;

				//Them khung hinh theo vet
				itPeople->rectsTrack.push_back(itPeople->lastRectangle);

				//Tinh lai histogram cua doi tuong
				colorHistogram.setThreshold(MY_THRESHOLD);
				MatND nHist = colorHistogram.getHistogram(rawCopyFrame, itPeople->lastRectangle);
				itPeople->hist_current = nHist;

			}
			else
			{

				//Match doi tuong
				findMatchTemplateSingle(rawFrame, *itPeople);
				//findMatchTemplateMulti(rawFrame, *itPeople);

				//Kiem tra va cham matchRectangle, lastRectangle
				if(utils.isImpact(itPeople->matchRectangle, itPeople->lastRectangle))
				{
					//Dat lai khung hinh cua doi tuong
					itPeople->prevRectangle = itPeople->matchRectangle;
					itPeople->lastRectangle = itPeople->matchRectangle;

					//Tinh lai histogram cua doi tuong
					colorHistogram.setThreshold(MY_THRESHOLD);
					MatND nHist = colorHistogram.getHistogram(rawCopyFrame, itPeople->lastRectangle);
					itPeople->hist_current = nHist;

					//Them khung hinh theo vet
					itPeople->rectsTrack.push_back(itPeople->matchRectangle);
				}
				else
				{
					itPeople = peopleContainerTrack.erase(itPeople);
					continue;
				}//end match doi tuong

			}

			//Ve hinh theo vet
			//cv::putText(rawFrame, "( "+this->intToString(itPeople->ID)+ ")", cv::Point(itPeople->lastRectangle.x, itPeople->lastRectangle.y), FONT_HERSHEY_PLAIN, 2, Scalar(0,255,0),1);
			cv::rectangle(rawFrame, itPeople->lastRectangle, cv::Scalar(0, 255, 0), 1);
			//utils.drawTrack(rawFrame, *itPeople, Scalar(0,0,255));

			//Kiểm tra xem có ra ngoài cửa hàng hay không
			if(!utils.isIn(itPeople->lastRectangle, roi))
			{
				//Tính thời gian của người đó
				time_t timeOut;
				time(&timeOut);
				double seconds = difftime(timeOut, itPeople->startTime);
				//Thời gian ở lại cửa hàng của khách hàng phải > 10 giây thì mới tính là 1 khách hàng.
				if(seconds >= MIN_TIMEIN)
				{
					string strTimeIn = connectDb.convertTimeInOut(itPeople->startTime);
					string strTimeOut = connectDb.convertTimeInOut(timeOut);
					string strCamDate = connectDb.createCAM_DATE(timeOut);

					string prefix_customer_cd =connectDb.createDateCustomerCd(itPeople->startTime);
					string maKhachHang = connectDb.createFormatCustomerId(SHOP_CD, prefix_customer_cd, index);

					(this->connectDb).insert_TB_CAM_MARKET_CSMR(this->db, SHOP_CD, strCamDate, maKhachHang, strTimeIn, strTimeOut, seconds);
					index++;
				}

				//Xóa khỏi danh sách peopleContainerTrack
				itPeople = peopleContainerTrack.erase(itPeople);
				continue;
			}

			itPeople++;
		}

		//Phat hien doi tuong chua duoc theo vet trong cua hang
		if(numBlobOfRoi > peopleContainerTrack.size())
		{
			vector<int> countTaken(contours.size(), 0);
			int cSize = contours.size();
			int pSize = peopleContainerTrack.size();			

			for(int i=0; i< pSize; i++)
			{
				Blob blob = peopleContainerTrack[i];
				int idex = -1;
			int mindis = 0 ;

				for(int j =0; j < cSize; j++)
				{
					Rect rect = cv::boundingRect(contours[j]);
					if(utils.isIn(rect, roi) && (countTaken[j]==0))
					{
						int dis = utils.getDistance(rect, blob.lastRectangle);
						if(dis < mindis)
						{
							idex = j;
							mindis = dis;
						}
					}
				}//end for

				//Kiem tra khoi gan blob nhat
				if(idex != -1)
				{
					countTaken[idex] = 1; //Danh dau da dung
				}
			}//end for pSize

			for(int i = 0; i< cSize; i++)
			{
				Rect rect = cv::boundingRect(contours[i]);
				if(utils.isIn(rect, roi) && countTaken[i]==0)
				{
					//Tao doi tuong moi
					Blob blob;
					blob.ID = ID++;
					blob.frameCount =1;
					blob.prevRectangle = rect;
					blob.lastRectangle = rect;
					blob.firstFrameNumber = frameNumber;
					blob.lastFrameNumber = frameNumber;
					blob.matchRectangle = rect;
					blob.frameROI = rawFrame(rect);
					blob.rectsTrack.push_back(rect);

					time_t timeIn;
					time(&timeIn);
					blob.startTime = timeIn;

					//Tinh histogram
					colorHistogram.setThreshold(MY_THRESHOLD);
					MatND nHist = colorHistogram.getHistogram(rawCopyFrame, rect);
					blob.hist_current = nHist;
					//cv::rectangle(rawFrame, bRect, Scalar(0,0,255));
					peopleContainerTrack.push_back(blob);
				}
			}

		}//end phat hien doi tuong chua duoc theo vet

		
		//Duyệt danh sách blobContainer
		int blobContainerSize = blobContainer.size();
		for(int i=0; i < blobContainerSize; i++)
		{
			//dự đoán khung hình 
			Blob blob = blobContainer[i];
			Rect rect = blob.lastRectangle;


			//Dung MeanShift du doan dich chuyen.
			meanShiftTracker.setTargetCurrent(rect);
			Rect fRect = meanShiftTracker.process(rawCopyFrame, blob.hist_current, MY_THRESHOLD);

			//Kiem tra khoang cach giua fRect va lastRectangle
			if(utils.isImpact(fRect, blob.lastRectangle))
			{
				//Hoan vi
				blob.frameROI = rawFrame(blob.lastRectangle);
				blob.prevRectangle = blob.lastRectangle;
				blob.lastRectangle = fRect;

				//Tinh lai histogram cua doi tuong
				colorHistogram.setThreshold(MY_THRESHOLD);
				MatND nHist = colorHistogram.getHistogram(rawCopyFrame, blob.lastRectangle);
				blob.hist_current = nHist;

			}
			else
			{
				//Dung templateMatch tim doi tuong
				findMatchTemplateSingle(rawFrame, blob);

				if(utils.isImpact(blob.matchRectangle, blob.lastRectangle))
				{
					//Dat lai khung hinh cua doi tuong
					blob.prevRectangle = blob.matchRectangle;
					blob.lastRectangle = blob.matchRectangle;

					//Tinh lai histogram cua doi tuong
					colorHistogram.setThreshold(MY_THRESHOLD);
					MatND nHist = colorHistogram.getHistogram(rawCopyFrame, blob.lastRectangle);
					blob.hist_current = nHist;
				}
				
			}


			if(utils.isIn(blob.lastRectangle, roi))
			{
				time_t timeIn;
				time(&timeIn);
				blob.startTime = timeIn;
				blob.lastFrameNumber = frameNumber;

				blob.rectsTrack.push_back(blob.lastRectangle);
				peopleContainerTrack.push_back(blob);
			}
		}//end duyet blobContainer

		//Xoa danh sach cu
		blobContainer.clear();

		int sizeContours = contours.size();
		//int num = 0;
		for(int i=0; i < sizeContours; i++)
		{
			bRect = cv::boundingRect(contours[i]);
			if(!utils.isIn(bRect, roi))
			{
				//MeanShiftTracker meanshift;
				Blob blob;
				blob.ID = ID++;
				blob.frameCount = 1;
				blob.prevRectangle = bRect;
				blob.lastRectangle = bRect;
				blob.firstFrameNumber = frameNumber;
				blob.lastFrameNumber = frameNumber;
				blob.matchRectangle = bRect;

				Mat v = rawFrame(bRect);
				blob.frameROI = v;

				//Tinh histogram
				colorHistogram.setThreshold(MY_THRESHOLD);
				MatND nHist = colorHistogram.getHistogram(rawCopyFrame, bRect);
				blob.hist_current = nHist;
				//cv::rectangle(rawFrame, bRect, Scalar(0,0,255));
				blobContainer.push_back(blob);
			}
		}


		imshow("Frame", rawFrame);
		//cout << "PeopleContainer: " << peopleContainerTrack.size() << endl;
		//cout << "BlobContainer: " << blobContainer.size() << endl;

		if (waitKey(10) > 0) {
			break;
		}
	}

	this->cap.release();
}
