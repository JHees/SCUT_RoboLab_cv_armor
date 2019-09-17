
#include "pch.h"
#include "Functions.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <boost/progress.hpp>
#include <vector>
#define is_Red(i) ((i>156)||(i<10))
#define is_Blue(i) (i>=90&&i<124)

//#define TARGET(i) is_Red(i)
#ifndef TARGET(i)
#define TARGET(i) is_Blue(i)
#endif

constexpr auto _VEDIO = "armor_2.mkv";
int main()
{
	cv::VideoCapture cap(_VEDIO);
	cv::namedWindow("R");
	int shold_median = 5;
	int shold = 16;
	cv::createTrackbar("shold: ", "R", &shold_median, 20);
	while (1)
	{
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
		imshow("frame", frame);

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
			myShold(channels_unB[2], Value_unB_sholded, [](int i)->uchar {return i >= 56 &&i<184 ? 255 : 0; });
			R = Hue_sholded & Value_unB_sholded ;
			
			cv::morphologyEx(R, R, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3), cv::Point(-1, -1)));
			cv::morphologyEx(R, R, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3), cv::Point(-1, -1)));
			std::cout << "morphology: ";
		}
		{
			cv::Mat con(R.rows, R.cols, CV_8UC1, cv::Scalar(0, 0, 0));
			cv::findContours(R, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_TC89_KCOS);
			for (std::vector<cv::Vec4i>::iterator i = hierarchy.begin(); i != hierarchy.end(); ++i)
			{
				if (i->val[0] == -1 && i->val[1] == -1 && i->val[2] == -1 && i->val[3] != -1)
				{
					contours_Tar.push_back(contours[std::distance(hierarchy.begin(), i)]);
				}
			}
			std::cout << "findContours: ";
			cv::drawContours(con, contours_Tar, -1, cv::Scalar(255, 255, 255));
			imshow("con", con);
		}
		
		imshow("R", R);
		std::cout << "global time: " << g_time.elapsed() << std::endl;;
		cv::waitKey((100 - 1000 * g_time.elapsed()) > 0 ? (100 - 1000 * g_time.elapsed()) : 1);
	}
}

