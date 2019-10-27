#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>
#include <boost/progress.hpp>
#include <vector>
#include<math.h>

class armor
{
public:
	const cv::Rect Rect1;
	const cv::Rect Rect2;
	cv::Point2f points[4];
    cv::Point2f center;
    cv::Size2f size;
public:
	armor() = default;
	armor(const cv::Rect&, const cv::Rect&);
	~armor() = default;
};


armor::armor(const cv::Rect& R1, const cv::Rect& R2)
{
    points[0].x=R1.tl().x<R2.tl().x?R1.tl().x:R2.tl().x;
    points[0].y=R1.tl().y<R2.tl().y?R1.tl().y:R2.tl().y;
    points[2].x=R1.br().x>R2.br().x?R1.br().x:R2.br().x;
    points[2].y=R1.br().y>R2.br().y?R1.br().y:R2.br().y;

    points[1].x=points[0].x;
    points[1].y=points[2].y;
    points[3].x=points[2].x;
    points[3].y=points[0].y;
    center = (points[0]+points[2])/2;
    size=points[2]-points[0];
}