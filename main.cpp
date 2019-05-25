#include<iostream>
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp> 
#include<opencv2/opencv.hpp>
#include<opencv2/objdetect.hpp>
#include"detect.h"
#include"Fatigue.h"
#include"Trans.h"
#include"histogram.h"
#include"ostuThreshold.h"

using namespace std;
using namespace cv;

#define DETECTTIME	30								// 一次检测过程的时间长度，用检测次数衡量
#define FATIGUETHRESHOLD	180						// 判断是否疲劳驾驶的阈值

int main() {
	
	int failFaceNum = 0;							// 统计一次检测过程中未检测到人脸的总数
	int failFaceDuration = 0;						// 统计一次检测过程中连续未检测到人脸的次数
	int maxFailFaceDuration = 0;					// 一次检测过程中连续未检测到人脸的次数的最大值
	int fatigueState = 1;							// 驾驶员的驾驶状态：疲劳驾驶（1），正常驾驶（0）
	int eyeState;									// 眼睛综合睁（0）、闭（1）状态
	int eyeCloseNum = 0;							// 统计一次检测过程中闭眼的总数
	int eyeCloseDuration = 0;						// 统计一次检测过程中连续检测到闭眼状态的次数
	int maxEyeCloseDuration = 0;					// 一次检测过程中连续检测到闭眼状态的次数的最大值
	int lEyeState;									// 左眼睁（0）、闭（1）状态
	int rEyeState;									// 右眼睁（0）、闭（1）状态
	int globalK = 0, globali = 0;
	int temp = 0;									//最大的人脸矩形区域面积
	cv::Rect* largestFaceRect = nullptr;			// 存储检测到的最大的人脸矩形框
	IplImage* srcImg = nullptr;						// 存放从摄像头读取的每一帧彩色源图像
	IplImage* img = nullptr;						// 存放从摄像头读取的每一帧灰度源图像
	IplImage* faceImg = nullptr;					// 存储检测出的人脸图像
	clock_t start, stop;							// 计时参数
	cv::Rect eyeRect;								// 存储裁剪后的人眼的矩形区域
	cv::Rect eyeRectTemp;							// 临时矩形区域
	IplImage* lEyeImg = nullptr;					// 存储左眼的图像
	IplImage* rEyeImg = nullptr;					// 存储右眼的图像
	int hist[256];									// 存放直方图的数组
	int pixelSum;									//二值化后图片像素的总数
	int threshold;									// 存储二值化最优阈值


	Detect *detect = new Detect();
	for (globalK; globalK <= DETECTTIME; globalK++) {
		start = clock();

		detect->startDetect();

		Mat tempcapimg;
		detect->getCap() >> tempcapimg;
		IplImage tempimg = tempcapimg;
		srcImg = cvCloneImage(&tempimg);

		Mat tempgrayimg;
		tempgrayimg.create(tempcapimg.size(), tempcapimg.type());
		cvtColor(tempcapimg, tempgrayimg, cv::COLOR_BGR2GRAY);
		IplImage tempgray = tempgrayimg;
		img = cvCloneImage(&tempgray);
		cvCvtColor(srcImg, img, CV_BGR2GRAY);
		
		/************************************* 检测人脸 ****************************************/
		cvWaitKey(20);

		// 提取人脸区域
		if ((detect->getFaces().size()) == 0) {
			cout << "没有获取到脸部" << endl;		
			failFaceNum++;							// 统计未检测到人脸的次数
			failFaceDuration++;						// 统计连续未检测到人脸的次数
			// 检测过程中判断全是闭眼和检测不到人脸的情况，没有睁开眼的情况
			(eyeCloseDuration > maxEyeCloseDuration) ? maxEyeCloseDuration = eyeCloseDuration : maxEyeCloseDuration;
			eyeCloseDuration = 0;					// 统计一次检测过程中连续检测到闭眼状态的次数
			if (globalK == DETECTTIME) {
				// 当一次检测过程中，都没有检测到人脸，则要在此更新 maxFailFaceDuration 
				(failFaceDuration > maxFailFaceDuration) ? maxFailFaceDuration = failFaceDuration : maxFailFaceDuration;

				cout << "达到疲劳状态" << endl;
				cout << "闭眼次数:" << eyeCloseNum << "最大连续闭眼次数:" << maxEyeCloseDuration << endl;
				cout << "获取脸部失败次数:" << failFaceNum << "最大连续获取脸部失败次数:" << maxFailFaceDuration << endl;

				// 进行疲劳状态的判别
				Fatigue *recoFatigueState = new Fatigue(FATIGUETHRESHOLD, eyeCloseNum, maxEyeCloseDuration, failFaceNum, maxFailFaceDuration);
				fatigueState = recoFatigueState->getcompreValue();
				if (fatigueState == 1) {
					cout << "驾驶员处于疲劳驾驶状态" << endl << endl;
				}
				else if (fatigueState == 0) {
					cout << "驾驶员处于正常驾驶状态" << endl << endl;
				}

				// 进入下一次检测过程前，将变量清零
				globalK = 0;
				lEyeState = 1;
				rEyeState = 1;
				eyeState = 1;
				eyeCloseNum = 0;
				eyeCloseDuration = 0;
				maxEyeCloseDuration = 0;
				failFaceNum = 0;
				failFaceDuration = 0;
				maxFailFaceDuration = 0;
				fatigueState = 1;

				cvWaitKey(1000);
			}
			continue;
		}
		else {//检测到脸部
			// 统计连续未检测到人脸的次数中的最大数值
			(failFaceDuration > maxFailFaceDuration) ? maxFailFaceDuration = failFaceDuration : maxFailFaceDuration;
			failFaceDuration = 0;
			// 找到检测到的最大的人脸矩形区域
			temp = 0;
			for (int i = 0; i < detect->getFaces().size(); i++) {
				if ((detect->getFaces().at(i).width * detect->getFaces().at(i).height) > temp) {
					temp = detect->getFaces().at(i).width * detect->getFaces().at(i).height;
					largestFaceRect = new Rect(detect->getFaces().at(i));
				}
			}

			// 根据人脸的先验知识分割出大致的人眼区域
			temp = largestFaceRect->width / 8;
			largestFaceRect->x = largestFaceRect->x + temp;
			largestFaceRect->width = largestFaceRect->width - 3 * temp / 2;
			largestFaceRect->height = largestFaceRect->height / 2;
			largestFaceRect->y = largestFaceRect->y + largestFaceRect->height / 2;
			largestFaceRect->height = largestFaceRect->height / 2;

			cvSetImageROI(img, *largestFaceRect);				// 设置ROI为检测到的最大的人脸区域
			faceImg = cvCreateImage(cvSize(largestFaceRect->width, largestFaceRect->height), IPL_DEPTH_8U, 1);
			cvCopy(img, faceImg, NULL);
			cvResetImageROI(img);								// 释放ROI
			cvShowImage("分割后的人眼区域", faceImg);
			//aitKey(0);

			eyeRectTemp = *largestFaceRect;
			// 根据人脸的先验知识分割出大致的左眼区域
			largestFaceRect->width /= 2;
			cvSetImageROI(img, *largestFaceRect);				// 设置ROI为检测到的最大的人脸区域
			lEyeImg = cvCreateImage(cvSize(largestFaceRect->width, largestFaceRect->height), IPL_DEPTH_8U, 1);
			cvCopy(img, lEyeImg, NULL);
			cvResetImageROI(img);								// 释放ROI
			cvShowImage("大致的左眼区域", lEyeImg);
			//waitKey(0);

			// 根据人脸的先验知识分割出大致的右眼区域
			eyeRectTemp.x += eyeRectTemp.width / 2;
			eyeRectTemp.width /= 2;
			cvSetImageROI(img, eyeRectTemp);					// 设置ROI为检测到的最大的人脸区域
			rEyeImg = cvCreateImage(cvSize(eyeRectTemp.width, eyeRectTemp.height), IPL_DEPTH_8U, 1);
			cvCopy(img, rEyeImg, NULL);
			cvResetImageROI(img);								// 释放ROI
			cvShowImage("大致的右眼区域", rEyeImg);
			//waitKey(0);

			/********************************** 二值化处理 ***********************************/
			cvSmooth(lEyeImg, lEyeImg, CV_MEDIAN);				// 中值滤波 默认窗口大小为3*3
			Trans nonlineTrans(lEyeImg, lEyeImg, 0.8);			// 非线性点运算
			memset(hist, 0, sizeof(hist));						// 初始化直方图的数组为0
			Histogram histogram(lEyeImg, hist);					// 计算图片直方图
			// 计算最佳阈值
			pixelSum = lEyeImg->width * lEyeImg->height;

			OstuThreshold ostuThreshold(hist, pixelSum, 45);
			threshold = ostuThreshold.getostu();
			cvThreshold(lEyeImg, lEyeImg, threshold, 255, CV_THRESH_BINARY);// 对图像二值化
			// 显示二值化后的图像
			cvShowImage("二值化后的眼睛", lEyeImg);
			waitKey(0);
		}
	}
	return 0;
}

