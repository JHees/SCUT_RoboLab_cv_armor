#include "include/Functions.h"
#include "include/armor.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <boost/timer.hpp>
#include <vector>
#include <cmath>
#define is_Red(i) ((i>156)||(i<10))
#define is_Blue(i) (i>=88&&i<=120)

//#define TARGET(i) is_Red(i)
#ifndef TARGET(i)

	#define TARGET(i) is_Blue(i)
#endif

constexpr auto _VEDIO = "../resource/armor_2.mkv";
int main()
{
	int flag_play = 10;
	cv::VideoCapture cap(_VEDIO);

	while (1)
	{

		cv::Mat reduce;
		cv::Mat HSV;
		std::vector<cv::Mat> channels_unB;
		cv::Mat Hue_sholded, Hue_unB_sholded, Value_unB_sholded;
		cv::Mat R;
		std::vector<std::vector<cv::Point>> contours, contours_Tar;
		std::vector<cv::Vec4i> hierarchy;
		std::vector<cv::Rect> rRect_Tar;
		switch (cv::waitKey(flag_play))
		{

			case 32:
			flag_play=flag_play==0?10:0;
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
		if(cap.get(cv::CAP_PROP_POS_FRAMES)==1100)
			cap.set(cv::CAP_PROP_POS_FRAMES,2300);
		if(cap.get(cv::CAP_PROP_POS_FRAMES)==4100)
			cap.set(cv::CAP_PROP_POS_FRAMES,5800);
		boost::timer g_time;
		cv::Mat frame;
		std::cout << cap.get(cv::CAP_PROP_POS_FRAMES)<<":";//2300
		cap >> frame;
		if (frame.empty())
		{
			cap.set(cv::CAP_PROP_POS_FRAMES,0);
			continue;
		}

			colorReduce(frame, reduce, 16);
			cvtColor(reduce, HSV, cv::COLOR_BGR2HSV);
			colorReduce(HSV,HSV,16);
			split(HSV, channels_unB);
			myShold(channels_unB[0], Hue_unB_sholded, [](int i)->uchar {return TARGET(i) ? 255 : 0; });
			//cv::medianBlur(Hue_unB_sholded, Hue_sholded, 3);
			myShold(channels_unB[2], Value_unB_sholded, [](int i)->uchar {return i >=136 ? 255 : 0; });
			R = Hue_unB_sholded & Value_unB_sholded ;
			//cv::morphologyEx(R, R, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3,3), cv::Point(-1, -1)));
			cv::morphologyEx(R, R, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3,3), cv::Point(-1, -1)));
			cv::morphologyEx(R, R, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3,3), cv::Point(-1, -1)));
			
			cv::Mat con(R.rows, R.cols, CV_8UC3, cv::Scalar(0, 0, 0));

			cv::findContours(R, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_TC89_L1);
			std::vector<cv::Vec4f> direc_Tar;
			for (std::vector<cv::Vec4i>::iterator i = hierarchy.begin(); i != hierarchy.end(); ++i)
			{
				if (i->val[2] == -1 && i->val[3] == -1)
				{
					cv::Vec4f buf_direc;//轮廓朝向
					cv::fitLine(contours[std::distance(hierarchy.begin(),i)],buf_direc,cv::DIST_L2,0,0.01,0.01);
					line(con,cv::Point(buf_direc.val[2],buf_direc.val[3]),cv::Point(10*buf_direc.val[0]+buf_direc.val[2],10*buf_direc.val[1]+buf_direc.val[3]),	cv::Scalar(255, 0, 0), 1);
					if(abs(buf_direc.val[1]/buf_direc.val[0])>0.7)
					{
						cv::Rect buf_rect = cv::boundingRect(contours[std::distance(hierarchy.begin(),i)]);
						if(buf_rect.height<buf_rect.width)
						{
							int buf=buf_rect.height;
							buf_rect.height=buf_rect.width;
							buf_rect.width=buf;
						}
						if(((float)buf_rect.height/buf_rect.width>2)
						||(buf_rect.area()<1600&&(float)buf_rect.height/buf_rect.width>1.3)
						||(buf_rect.area()<50&&buf_rect.area()>10&&(float)buf_rect.height/buf_rect.width>1.01))
						{
							if(buf_direc.val[1]<0)
							{
								buf_direc.val[0]=-buf_direc.val[0];
								buf_direc.val[1]=-buf_direc.val[1];
							}
							direc_Tar.push_back(buf_direc);
							rRect_Tar.push_back(buf_rect);
							contours_Tar.push_back(contours[std::distance(hierarchy.begin(), i)]);
						}
						assert(direc_Tar.size()==contours_Tar.size());
					// 	else
					// 	{
					// 		cv::putText(con,"r",cv::Point(buf_direc.val[2],buf_direc.val[3]),cv::FONT_HERSHEY_PLAIN,1,cv::Scalar(255,255,255));
					// 	}
					 }
					// else
					// {
					// 		cv::putText(con,"d",cv::Point(buf_direc.val[2],buf_direc.val[3]),cv::FONT_HERSHEY_PLAIN,1,cv::Scalar(255,255,255));
					// }
				}
			}
			std::vector<cv::Vec3f> buf_dis;
			std::vector<bool> is_picked(direc_Tar.size(),0);
			for(auto i=direc_Tar.begin();i!=direc_Tar.end();++i)
			{
				for(auto j =i + 1;j!=direc_Tar.end();++j)
				{
					int x = std::distance(direc_Tar.begin(),i);
					int y = std::distance(direc_Tar.begin(),j);
					if(abs((i->val[3]-j->val[3])/(i->val[2]-j->val[2]))<0.7
					&&abs( rRect_Tar[x].area()-rRect_Tar[y].area())<50)
						buf_dis.push_back(cv::Vec3f(std::sqrt(	pow(i->val[3]-j->val[3],2)
																										+0.25*pow( cv::contourArea(contours_Tar[x])-cv::contourArea(contours_Tar[y]),2)
																										+0.25*pow(i->val[2]-j->val[2],2)
																										//+pow((direc_Tar[x].val[3]-direc_Tar[y].val[3])-(direc_Tar[x].val[2]-direc_Tar[y].val[2]),2)
																										),x,y));
				}
			}
			auto cmp=[](cv::Vec3f n1,cv::Vec3f n2){return n1.val[0]<n2.val[0];};
			std::sort(buf_dis.begin(),buf_dis.end(),cmp);
			std::vector<armor> Tar;
			for(auto i:buf_dis)
			{
				if(!(is_picked[i.val[1]])&&!(is_picked[i.val[2]]))
				{
						armor  buf_armor(rRect_Tar[i.val[1]],rRect_Tar[i.val[2]]);
						 if((buf_armor.size.width/buf_armor.size.height>1.5&&buf_armor.size.width/buf_armor.size.height<6)
						 && (rRect_Tar[i.val[1]].area()/rRect_Tar[i.val[2]].area()<1.2||rRect_Tar[i.val[2]].area()/rRect_Tar[i.val[1]].area()<1.2)
						 &&buf_armor.size.area()/std::sqrt(rRect_Tar[i.val[1]].area()*rRect_Tar[i.val[2]].area())<20)
						{
							int buf =0;
							for(auto j: direc_Tar)
							{

								if(cv::Rect(buf_armor.points[0],buf_armor.points[2]).contains(cv::Point(j.val[2],j.val[3])))
								{
									buf+=1;
								}
								
							}
							if(buf<=2)
							{
								Tar.push_back(buf_armor);
								is_picked[i.val[1]]=1;
								is_picked[i.val[2]]=1;
							}
						}
				}
			}
			for (auto i : direc_Tar)
			{
				for (int j = 0; j < 4; ++j)
				{
					line(con,cv::Point(i.val[2],i.val[3]),cv::Point(10*i.val[0]+i.val[2],10*i.val[1]+i.val[3]),	cv::Scalar(0, 255, 0), 1);
				}
			}
			for(auto i:Tar)
			{

				for (int j=0;j<4;++j)
				{
					line(frame,i.points[j],i.points[(j+1)%4],	cv::Scalar(0, 255, 0), 1);
				}
			}

			cv::drawContours(con, contours_Tar, -1, cv::Scalar(255, 255, 255),1);
			imshow("HSV",HSV);
			imshow("con", con);
			imshow("frame", frame);
			
			//imshow("R", R);
		 std::cout <<g_time.elapsed()<<std::endl;
	}
}

