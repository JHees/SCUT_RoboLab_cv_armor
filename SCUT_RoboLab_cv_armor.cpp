#include "Functions.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <boost/timer.hpp>
#include <vector>
#include <cmath>
#define is_Red(i) ((i>156)||(i<10))
#define is_Blue(i) (i>=90&&i<124)

//#define TARGET(i) is_Red(i)
#ifndef TARGET(i)

	#define TARGET(i) is_Blue(i)
#endif

constexpr auto _VEDIO = "../resource/armor_2.mkv";
int main()
{
	int flag_play = 100;
	cv::VideoCapture cap(_VEDIO);

	while (1)
	{
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
		std::vector<cv::RotatedRect> rRect,rRect_Tar;
		std::vector<armor> rRect_match;
		switch (cv::waitKey(flag_play))
		{

			case 32:
			flag_play=flag_play==0?100:0;
			break;

			case 100:
			cap.set(cv::CAP_PROP_POS_FRAMES,cap.get(cv::CAP_PROP_POS_FRAMES));
			break;

			case 97:
			cap.set(cv::CAP_PROP_POS_FRAMES,cap.get(cv::CAP_PROP_POS_FRAMES)-2);
			break;
			default:
			break;
		}
		if (frame.empty())
		{
			cap.set(cv::CAP_PROP_POS_FRAMES,0);
			continue;
		}

			colorReduce(frame, reduce, 16);
			cvtColor(reduce, HSV, cv::COLOR_BGR2HSV);
			split(HSV, channels_unB);
			myShold(channels_unB[0], Hue_unB_sholded, [](int i)->uchar {return TARGET(i) ? 255 : 0; });
			cv::medianBlur(Hue_unB_sholded, Hue_sholded, 3);
			myShold(channels_unB[2], Value_unB_sholded, [](int i)->uchar {return i >= 120 ? 255 : 0; });
			R = Hue_sholded & Value_unB_sholded ;
			cv::morphologyEx(R, R, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3), cv::Point(-1, -1)));
			cv::morphologyEx(R, R, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3), cv::Point(-1, -1)));
			
			cv::Mat con(R.rows, R.cols, CV_8UC3, cv::Scalar(0, 0, 0));

			cv::findContours(R, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_TC89_L1);
			for (std::vector<cv::Vec4i>::iterator i = hierarchy.begin(); i != hierarchy.end(); ++i)
			{
				if (i->val[2] == -1 && i->val[3] == -1)
				{
					contours_Tar.push_back(contours[std::distance(hierarchy.begin(), i)]);
				}
			}
			cv::drawContours(con, contours_Tar, -1, cv::Scalar(255, 255, 255));

			//cv::morphologyEx(con, con, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));


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


			for (auto i = rRect_Tar.begin(); i != rRect_Tar.end(); ++i)
			{
				if (i->angle == 0)
					continue;
				float ang = 0.5;
				int dis = 200;
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

			cv::drawContours(con, contours_Tar, -1, cv::Scalar(255, 255, 255),1);
			imshow("con", con);
			imshow("frame", frame);
			imshow("HSV",HSV);
			//imshow("R", R);
		 std::cout << cap.get(cv::CAP_PROP_POS_FRAMES)<<":"<<g_time.elapsed()<<std::endl;
	}
}

