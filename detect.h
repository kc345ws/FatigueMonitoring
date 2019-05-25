#pragma once
#ifndef DETECT_H
#define DETECT_H

#include<opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp> 
#include<opencv2/highgui/highgui_c.h>
#include <opencv2/core/core.hpp>
#include<vector>
using namespace cv;
using namespace std;
//脸部 眼部检测
class Detect {
private:
	VideoCapture cap;
	cv::CascadeClassifier face_cascade;
	//face_cascade.load("Haar\\haarcascade_frontalface_default.xml");
	cv::CascadeClassifier eye_cascade;
	//eye_cascade.load("Haar\\haarcascade_eye_tree_eyeglasses.xml");

	//IplImage* srcImg;					// 灰度图像
	//CvSeq* objects;					// 输出参数：检测到人脸的矩形框
	//CvMemStorage* storage;			// 存储矩形框的内存区域
	Mat img;//原图
	Mat gray;//灰度图
	vector<Rect> faces;//脸部向量
	vector<Rect>eyes;//眼部向量
	double scale_factor = 1.3;		// 搜索窗口的比例系数
	int min_neighbors = 5;			// 构成检测目标的相邻矩形的最小个数
	int flags = 0;					// 操作方式
	//CvSize min_size = cvSize(20, 20);// 检测窗口的最小尺寸

	Scalar colors[7] = {
		// 红橙黄绿青蓝紫		
		CV_RGB(255, 0, 0),
		CV_RGB(255, 97, 0),
		CV_RGB(255, 255, 0),
		CV_RGB(0, 255, 0),
		CV_RGB(0, 255, 255),
		CV_RGB(0, 0, 255),
		CV_RGB(160, 32, 240)
	};

public:
	Detect();
	void startDetect();//开始检测
	bool checkisOpened();//检测摄像头是否打开
	void detectFace();//检测脸部
	void detectEyes(const int index);//检测眼睛
	void showDetect();//显示检测图像
	vector<Rect> getFaces() { return faces; }
	VideoCapture getCap() { return cap; }
};
#endif 
