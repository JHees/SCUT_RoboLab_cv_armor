
#include "pch.h"
#include "Functions.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <boost/progress.hpp>
#include <vector>
#include <cmath>
#define is_Red(i) ((i>156)||(i<10))
#define is_Blue(i) (i>=90&&i<124)

//#define TARGET(i) is_Red(i)
#ifndef TARGET(i)

	#define TARGET(i) is_Blue(i)
#endif

constexpr auto _VEDIO = "armor_2.mkv";
int main()
{
	int n = 0;
	cv::VideoCapture cap(_VEDIO);
	cv::namedWindow("R");
	int shold_median = 5;
	int shold = 16;
	cv::createTrackbar("shold: ", "R", &shold_median, 20);
	while (1)
	{
		++n;
		std::cout << "\n------------------\n";
		boost::timer g_time;
		cv::Mat frame;
		cap >> frame;
		cv::Mat reduce;
		cv::Mat HSV;
		std::vector<cv::Mat> channels_unB;
		cv::Mat Hue_sholded, Hue_unB_sholded, Value_unB_sholded;
		cv::Mat R;
		std::vector<std::vector<cv::Point>> contours, contours_Tar;
		std::vector<cv::Vec4i> hierarchy;

		std::cout << "Create: " << g_time.elapsed() << std::endl;
		if (frame.empty())
		{
			cap.release();
			cap.open(_VEDIO);
			continue;
		}

		{
			boost::progress_timer t;
			colorReduce(frame, reduce, 16);
			cvtColor(reduce, HSV, cv::COLOR_BGR2HSV);
			
			std::cout << "colorReduce: ";
		}

		{
			boost::progress_timer t;
			split(HSV, channels_unB);
			myShold(channels_unB[0], Hue_unB_sholded, [](int i)->uchar {return TARGET(i) ? 255 : 0; });
			cv::medianBlur(Hue_unB_sholded, Hue_sholded, 3);
			myShold(channels_unB[2], Value_unB_sholded, [](int i)->uchar {return i >= 120 ? 255 : 0; });
			R = Hue_sholded & Value_unB_sholded ;

			cv::morphologyEx(R, R, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3), cv::Point(-1, -1)));
			cv::morphologyEx(R, R, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3), cv::Point(-1, -1)));
			std::cout << "morphology: ";
		}
		//{
			cv::Mat con(R.rows, R.cols, CV_8UC3, cv::Scalar(0, 0, 0));
			cv::findContours(R, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_TC89_L1);
			for (std::vector<cv::Vec4i>::iterator i = hierarchy.begin(); i != hierarchy.end(); ++i)
			{
				if (i->val[2] == -1 && i->val[3] == -1)
				{
					contours_Tar.push_back(contours[std::distance(hierarchy.begin(), i)]);
				}
			}
			std::cout << "findContours: ";
			cv::drawContours(con, contours_Tar, -1, cv::Scalar(255, 255, 255));
			
		//}
			//cv::morphologyEx(con, con, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
		std::vector<cv::RotatedRect> rRect,rRect_Tar;
		for (auto i : contours_Tar)
		{
			rRect.push_back(cv::minAreaRect(i));
		}
		
		for (auto &i : rRect)
		{
			if (i.size.width < i.size.height)
			{
				i.angle += (i.angle < 0 ? 90 : -90);
				i.angle += i.angle < 0 ? 180 : 0;
				int buf = i.size.width;
				i.size.width = i.size.height;
				i.size.height = buf;
			}
			if ((i.angle > 0 ? (i.angle > 44 && i.angle <136) : (-i.angle > 43 && -i.angle < 136)))//&&((i.size.width/i.size.height>2&& i.size.width / i.size.height<3)|| (i.size.height / i.size.width > 2 && i.size.height / i.size.width < 3)))
			{
				rRect_Tar.push_back(i);
			}
			cv::Point2f vertices[4];
			i.points(vertices);
			for (int j = 0; j < 4; j++)
			{
				//if ((i.angle > 0 ? (i.angle > 50 && i.angle < 130) : (-i.angle > 50 && -i.angle < 130)))//&&((i.size.width/i.size.height>2&& i.size.width / i.size.height<3)|| (i.size.height / i.size.width > 2 && i.size.height / i.size.width < 3)))
					//line(frame, vertices[j], vertices[(j + 1) % 4], cv::Scalar(179, 245, 222), 1);				
				line(con, vertices[j], vertices[(j + 1) % 4], cv::Scalar(179, 245, 222), 1);
			}
		}
		std::vector<armor> rRect_match;
		
		for (auto i = rRect_Tar.begin(); i != rRect_Tar.end(); ++i)
		{
			if (i->angle == 0)
				continue;
			float ang = 0.5;
			int dis = 150;
			auto buf = i;
			for (auto j = i + 1; j != rRect_Tar.end(); ++j)
			{
				if (dis > distance(i->center, j->center)
					&& ang > (abs((i->center.y - j->center.y) / (i->center.x - j->center.x)))
					&& (distance(i->center, j->center) / (i->size.width + j->size.width)) < 5
					&& (i->size.width / j->size.width < 1.3 && j->size.width / i->size.width < 1.3))
				{
					ang = (abs((i->center.y - j->center.y) / (i->center.x - j->center.x)));
					dis = distance(i->center, j->center);
					buf = j;
				}
			}
			if (buf != i)
			{

				//line(frame, i->center, buf->center, cv::Scalar(0, 255, 0), 1);
				rRect_match.push_back(armor(*i, *buf));
					buf->angle = 0;
			}
		}
		for (auto i : rRect_match)
		{
			for (int j = 0; j < 4; ++j)
			{
				line(frame, i.points[j], i.points[(j + 1) % 4], cv::Scalar(0, 255, 0), 1);
			}
		}

			//std::vector<cv::Vec4i>contours_bou;
			//cv::HoughLinesP(con, contours_bou, 1,3.1415/180, 3, 3,0);
			//for (auto i : contours_bou)
			//{
			//	line(frame, cv::Point(i.val[0],i.val[1]), cv::Point(i.val[2],i.val[3]), cv::Scalar(179, 245, 222), 1);
			//	
			//}
			//std::vector<std::vector<cv::Point>> contours_dire;
			//for (auto i : contours_Tar)
			//{
			//	int dis = 0;
			//	auto mp1 = i.begin(), mp2 = i.begin();
			//	for (auto p1 = i.begin(); p1 != i.end(); ++p1)
			//	{
			//		for (auto p2 = p1 + 1; p2 != i.end(); ++p2)
			//		{
			//			int buf = (p1->x - p2->x)*(p1->x - p2->x) + (p1->y - p2->y)*(p1->y - p2->y);
			//			if (dis < buf)
			//			{
			//				dis = buf;
			//				mp1 = p1; 
			//				mp2 = p2;
			//			}
			//			
			//		}
			//	}
			//	std::vector<cv::Point> a = { *mp1,*mp2 };
			//	//a.push_back(*mp1);
			//	//a.push_back(*mp2);
			//	contours_dire.push_back(a);
			//}
			//for (auto i : contours_dire)
			//{
			//	line(frame, i[0], i[1], cv::Scalar(179, 245, 222), 1);
			//}

		cv::drawContours(con, contours_Tar, -1, cv::Scalar(255, 255, 255),1);
		imshow("con", con);
		imshow("frame", frame);

		 
		imshow("R", R);
		std::cout << "global time: " << g_time.elapsed() << std::endl;;
		std::cout << n << std::endl;
		cv::waitKey((100 - 1000 * g_time.elapsed()) > 0 ? (100 - 1000 * g_time.elapsed()) : 1);
	}
}

