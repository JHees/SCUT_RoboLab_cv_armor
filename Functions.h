#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>
#include <boost/progress.hpp>
#include <vector>
#include<math.h>
class armor
{
public:
	const cv::RotatedRect Rect1;
	const cv::RotatedRect Rect2;
	cv::Point2f points[4];
	float angle;
public:
	armor() = default;
	armor(const cv::RotatedRect&, const cv::RotatedRect&);
	~armor() = default;
};
armor::armor(const cv::RotatedRect& R1, const cv::RotatedRect& R2)
{
	cv::Point2f ver[8];
	R1.points(ver);
	R2.points(ver+4);
	points[0] = points[1] = points[2] = points[3] = ver[0];
	for (int i = 0; i < 8; ++i)
	{
		points[0] = cv::Point2f(points[0].x < ver[i].x ? points[0].x : ver[i].x, points[0].y < ver[i].y ? points[0].y : ver[i].y);
		points[1] = cv::Point2f(points[1].x > ver[i].x ? points[1].x : ver[i].x, points[1].y < ver[i].y ? points[1].y : ver[i].y);
		points[2] = cv::Point2f(points[2].x > ver[i].x ? points[2].x : ver[i].x, points[2].y > ver[i].y ? points[2].y : ver[i].y);
		points[3] = cv::Point2f(points[3].x < ver[i].x ? points[3].x : ver[i].x, points[3].y > ver[i].y ? points[3].y : ver[i].y);
	}
}

inline double distance(cv::Point p1, cv::Point p2)
{
	return std::sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}
void colorReduce(const cv::Mat& input, cv::Mat& output, int div)
{
	cv::Mat Table(1, 256, CV_8U);
	uchar* p = Table.data;
	for (int i = 0; i < 256; ++i)
	{
		p[i] = i / div * div + div / 2;
	}
	cv::LUT(input, Table, output);
}
void myShold(const cv::Mat &mat1, cv::Mat &mat2, uchar(*tb)(int))
{
	cv::Mat Table(1, 256, CV_8U);
	auto p = Table.data;
	for (int i = 0; i < 256; ++i)
	{
		p[i] = tb(i);
	}
	cv::LUT(mat1, Table, mat2);
};

//p2 p3 p4
//p5 p1 p6
//p7 p8 p9
//
//void mySkeleton(const cv::Mat &input, cv::Mat &output)
//{
//	cv::Mat ip = input.clone();
//	int ip_row = ip.rows;
//	int ip_col = ip.cols;
//	std::vector<uchar*> mkdel;
//	while(true)
//	{
//		cv::Mat ip_canny;
//		cv::Mat ip_morph;
//		cv::Canny(ip, ip_canny, 254, 253);
//		cv::morphologyEx(ip, ip_morph, cv::MORPH_TOPHAT, cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3)));
//		for (int i = 0; i < ip_row; ++i)
//		{
//			for (int j = 0; j < ip_col; ++j)
//			{
//				bool p1 = ip.ptr<uchar>(i)[j];
//				if (!p1) continue;
//				bool p2 = (i == 0 || j == 0) ? 0 : ip.ptr<uchar>(i - 1)[j - 1];
//				bool p3 = (i == 0) ? 0 : ip.ptr<uchar>(i - 1)[j];
//				bool p4 = (i == 0 || j > ip_col - 2) ? 0 : ip.ptr<uchar>(i - 1)[j + 1];
//				bool p5 = (j == 0) ? 0 : ip.ptr<uchar>(i)[j - 1];
//				bool p6 = (j > ip_col - 2) ? 0 : ip.ptr<uchar>(i)[j + 1];
//				bool p7 = (i > ip_row - 2 || j == 0) ? 0 : ip.ptr<uchar>(i + 1)[j - 1];
//				bool p8 = (i > ip_row - 2) ? 0 : ip.ptr<uchar>(i + 1)[j];
//				bool p9 = (i > ip_row - 2 || j > ip_col - 2) ? 0 : ip.ptr<uchar>(i + 1)[j + 1];
//
//				if (ip_canny.ptr<uchar>(i)[j] && (!ip_morph.ptr<uchar>(i)[j] && ((int)p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && ((int)p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6))
//				{
//					mkdel.push_back(ip.ptr<uchar>(i) + j);
//				}
//			}
//		}
//		for (auto i : mkdel)
//		{
//			*i = 0;
//		}
//		//imshow("233", ip);
//		//imshow("234", ip_canny);
//		//cv::waitKey();
//		
//
//		if (mkdel.empty())
//		{
//			output = ip.clone();
//			break;
//		}
//		else
//		{
//			mkdel.clear();
//			continue;
//		}
//			
//	}
//
//
//}