#include "stdafx.h"
#include "Tracker.h"

using namespace cv;
using namespace std;

//>>>Thời gian trong cửa hàng 
#define MIN_TIMEIN 10.0
//<<<Thời gian trong cửa hàng

size_t CTrack::NextTrackID = 1;

CTrack::CTrack(Point2f pt, float dt, float Accel_noise_mag)
{
	track_id = NextTrackID;

	NextTrackID++;
	KF = new TKalmanFilter(pt, dt, Accel_noise_mag);

	//Khởi tạo biến tính thời gian false
	is_calc_time = false;

	prediction = pt;
	skipped_frames = 0;
}

CTrack::~CTrack()
{
	delete KF;
}


CTracker::CTracker(float _dt, float _Accel_noise_mag, double _dist_thres, int _maximum_allowed_skipped_frames,int _max_trace_length)
{
	dt=_dt;
	Accel_noise_mag=_Accel_noise_mag;
	dist_thres=_dist_thres;
	maximum_allowed_skipped_frames=_maximum_allowed_skipped_frames;
	max_trace_length=_max_trace_length;
}

bool CTracker::isIn(cv::Point2d center, const cv::Rect& roi)
{

	double x = center.x;
	double y = center.y;

	double roiTopLeftX = roi.x;
	double roiTopLeftY = roi.y;
	double roiBottomLeftY = roi.y + roi.height;
	double roiTopRightX = roi.x +  roi.width;

	if((x >=roiTopLeftX)&&(x <= roiTopRightX)&&(y >= roiTopLeftY)&&(y <= roiBottomLeftY))
	{
		return true;
	}

	return false;
}

//>>>Cập nhật vị trí theo vết
//Đầu vào là một tập điểm đại diện cho đối tượng được phát hiện

void CTracker::Update(vector<Point2d>& detections, const cv::Rect& roi)
{
	// -----------------------------------
	// Nếu các điểm chưa được theo vết
	// -----------------------------------
	if(tracks.size()==0)
	{
		// Nếu chưa được theo vết
		for(int i=0; i < detections.size(); i++)
		{
			//Tạo đối tượng mới
			CTrack* tr = new CTrack(detections[i], dt, Accel_noise_mag);

			//Kiểm tra điểm phát hiện đối tượng lần đầu tiên có trong cửa hàng không
			if(isIn(detections[i], roi))
			{
				tr->is_calc_time = true;
				//Lấy thời gian hiện tại
				time_t curr;
				time(&curr);
				tr->timestart = curr;

				cout << "Vao cua hang" << endl;
			}

			tracks.push_back(tr);
		}	
	}

	int N=tracks.size();		
	int M= detections.size();	

	vector< vector<double> > Cost(N ,vector<double>(M));
	vector<int> assignment; 

	//Tính ma trận chi phí
	double dist;
	for(int i=0; i < tracks.size();i++)
	{	

		for(int j=0; j < detections.size();j++)
		{
			//Tính độ sai khác của dự đoán và phát hiện
			Point2d diff=(tracks[i]->prediction - detections[j]);
			dist = sqrtf(diff.x*diff.x + diff.y*diff.y);

			//Chi phí cho khoảng cách này
			Cost[i][j]=dist;
		}
	}

	// -----------------------------------
	//Gán điểm theo thuật toán Hungarian
	// -----------------------------------
	AssignmentProblemSolver APS;
	APS.Solve(Cost, assignment, AssignmentProblemSolver::optimal);

	// -----------------------------------
	//Xóa các cặp gán có khoảng cách quá rộng
	// -----------------------------------
	vector<int> not_assigned_tracks;

	for(int i=0; i < assignment.size(); i++)
	{
		if(assignment[i]!=-1)
		{
			if(Cost[i][assignment[i]] > dist_thres) //Vượt ngưỡng cho phép
			{
				assignment[i]=-1;
				//Đánh dấu không được theo vết
				not_assigned_tracks.push_back(i);
			}
		}
		else
		{			
			// Tăng số khung hình bị bỏ qua
			tracks[i]->skipped_frames++;
		}

	}

	// -----------------------------------
	//Nếu theo vết bị mất trong một thời gian dài thì xóa đi 
	// -----------------------------------
	for(int i=0;i < tracks.size();i++)
	{
		if(tracks[i]->skipped_frames > maximum_allowed_skipped_frames)
		{
			delete tracks[i];
			tracks.erase(tracks.begin()+i);
			assignment.erase(assignment.begin()+i);
			i--;
		}
	}

	// -----------------------------------
	//Tìm kiếm các phát hiện chưa được gán
	// -----------------------------------
	vector<int> not_assigned_detections;
	vector<int>::iterator it;
	for(int i=0; i < detections.size(); i++)
	{
		it = find(assignment.begin(), assignment.end(), i);

		if(it==assignment.end())
		{
			not_assigned_detections.push_back(i);
		}
	}

	// -----------------------------------
	// Và bắt đầu một theo vết mới cho nó
	// -----------------------------------
	if(not_assigned_detections.size()!=0)
	{
		for(int i=0;i<not_assigned_detections.size();i++)
		{
			CTrack* tr=new CTrack(detections[not_assigned_detections[i]], dt, Accel_noise_mag);

			//Kiểm tra điểm phát hiện đối tượng lần đầu tiên có trong cửa hàng không
			if(isIn(detections[i], roi))
			{
				tr->is_calc_time = true;
				//Lấy thời gian hiện tại
				time_t curr;
				time(&curr);
				tr->timestart = curr;

				cout << "Vao cua hang" << endl;
			}

			tracks.push_back(tr);
		}	
	}

	//Cập nhật trạng thái với Kalman Filters state
	for(int i=0; i < assignment.size(); i++)
	{
		// If track updated less than one time, than filter state is not correct.
		tracks[i]->KF->GetPrediction();

		if(assignment[i]!=-1) // If we have assigned detect, then update using its coordinates,
		{
			tracks[i]->skipped_frames = 0; //Đặt lại số khung bỏ qua về 0
			tracks[i]->prediction = tracks[i]->KF->Update(detections[assignment[i]], 1); //Nếu phát hiện thì cập nhật bằng measurements
		}else // Nếu đối tượng không liên tục thì dự đoán
		{
			tracks[i]->prediction=tracks[i]->KF->Update(Point2f(0,0), 0);	
		}


		if(tracks[i]->trace.size() > 0)
		{
			//>>>Kiểm tra đối tượng có vào cửa hàng hay không
			if(!isIn((tracks[i]->trace).back(), roi) && isIn(tracks[i]->prediction, roi))
			{
				//Bắt đầu tính thời gian
				tracks[i]->is_calc_time = true;
				time_t curr;
				time(&curr);
				tracks[i]->timestart = curr;

				cout << "Vao cua hang" << endl;
			}
			//<<<Kiểm tra đối tượng có vào cửa hàng hay không

			//>>>Kiểm tra đối tượng có đi ra cửa hàng hay không
			if(isIn((tracks[i]->trace).back(), roi) && (!isIn(tracks[i]->prediction, roi)))
			{
				// Tính thời gian
				if(tracks[i]->is_calc_time)
				{
					tracks[i]->is_calc_time = false;
					time_t curr;
					time(&curr);
					double seconds = difftime(curr, tracks[i]->timestart);
					if(seconds >= MIN_TIMEIN)
					{
						cout << "Luu vao database" << endl;
					}
				}

			}
			//<<<Kiểm tra đối tượng có đi ra cửa hàng hay không
		}

		//Xóa đi số điểm vượt quá ngưỡng qui định trong dãy theo vết
		if(tracks[i]->trace.size() > max_trace_length)
		{
			tracks[i]->trace.erase(tracks[i]->trace.begin(),tracks[i]->trace.end()- max_trace_length);
		}

		tracks[i]->trace.push_back(tracks[i]->prediction);
		tracks[i]->KF->LastResult=tracks[i]->prediction;
	}

}

CTracker::~CTracker(void)
{
	for(int i=0;i<tracks.size();i++)
	{
		delete tracks[i];
	}
	tracks.clear();
}
