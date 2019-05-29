#include"detect.h"
#include<vector>
#include"Trans.h"
#include"histogram.h"
#include"ostuThreshold.h"

using namespace std;

Detect::Detect()
{
	cap = VideoCapture(0);
	face_cascade.load("Haar\\haarcascade_frontalface_default.xml");
	eye_cascade.load("Haar\\haarcascade_eye_tree_eyeglasses.xml");
}

void Detect::startDetect()
{
	if (checkisOpened()) {
		detectFace();
		showDetect();
	}
}

bool Detect::checkisOpened()
{
	if (cap.isOpened()) {
		return true;
	}
	return false;
}

void Detect::detectFace()
{
	//Mat img;
	cap >> img;
	//bool ret = cap.read(img);
	//Mat gray;
	gray.create(img.size(), img.type());
	cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
	//vector<Rect> faces;
	face_cascade.detectMultiScale(gray, faces, scale_factor, min_neighbors, flags);
	//cout << "检测到人脸个数:" << faces.size() << endl;
	for (int i = 0; i < faces.size(); i++) {
		Point Center;
		int radius;//半径
		Center.x = cvRound(faces[i].x + faces[i].width * 0.5);
		Center.y = cvRound(faces[i].y + faces[i].height * 0.5);
		radius = cvRound((faces[i].width + faces[i].height) * 0.25);

		circle(img, Center, radius, colors[5]);

		detectEyes(i);//检测眼睛
		detectMouth(gray,i);
		detectEyeswithoutHaar(i);
	}
	/*imshow("疲劳驾驶检测", img);
	if (cv::waitKey(1) && 1 == 2) {
		break;
	}*/	
}

void Detect::detectEyes(const int index)
{
	Mat faceROI = gray(faces[index]);
	//vector<Rect>eyes;
	eye_cascade.detectMultiScale(faceROI, eyes, 1.3, 3, 0);
	//cout << "检测到眼睛个数:" << eyes.size() << endl;
	for (int j = 0; j < eyes.size(); j++) {
		//Rect rect(faces[i].x + eyes[j].x, faces[i].y + eyes[j].y, eyes[j].width, eyes[j].height);
		//rectangle(img, rect, colors[3], 2, 8, 0);
		Point Center;
		int radius;
		Center.x = cvRound(faces[index].x + eyes[j].x + eyes[j].width * 0.5);
		Center.y = cvRound(faces[index].y + eyes[j].y + eyes[j].height * 0.5);
		radius = cvRound(eyes[j].width + eyes[j].height) * 0.25;
		circle(img, Center, radius, colors[3]);
	}
}

void Detect::detectMouth(Mat grayimg, const int index)//检测嘴部
{
	int hist[256];									// 存放直方图的数组
	int pixelSum;									// 二值化后图片像素的总数
	int threshold;									// 存储二值化最优阈值
	// 根据人脸的先验知识分割出大致的嘴部区域
	Rect MyMouTh = faces.at(index);
	//Rect MyMouTh = MyFace;
	//Center.x = cvRound(MyMouTh.x + faces[index].width * 0.15);
	//Center.y = cvRound(MyMouTh.y + faces[index].height * 0.01);
	float TempWidth = MyMouTh.width / 3;//将脸部分为3部分
	MyMouTh.x = MyMouTh.x + TempWidth;//嘴部的起始X坐标
	//MyMouTh.height / 3
	MyMouTh.y = faces[index].y + faces[index].height*0.72;//嘴部起始Y坐标
	//MyMouTh.width = MyMouTh.width - 3 * TempWidth / 2;
	MyMouTh.width = TempWidth + faces[index].width * 0.1;//嘴部矩形宽度
	MyMouTh.height = MyMouTh.height / 6 + faces[index].height * 0.001;//嘴部矩形高度
	//MyMouTh.height = MyMouTh.height / 2;

	IplImage Tempgray = grayimg;
	cvSetImageROI(&Tempgray, MyMouTh);				// 设置ROI为检测到的嘴部区域
	IplImage* mouthImg;
	mouthImg = cvCreateImage(cvSize(MyMouTh.width, MyMouTh.height), IPL_DEPTH_8U, 1);
	cvCopy(&Tempgray, mouthImg, NULL);
	cvResetImageROI(&Tempgray);								// 释放ROI
	cvShowImage("嘴部", mouthImg);

	/********************************** 二值化处理 ***********************************/
	cvSmooth(mouthImg, mouthImg, CV_MEDIAN);				// 中值滤波 默认窗口大小为3*3
	Trans nonlineTrans(mouthImg, mouthImg, 0.8);			// 非线性点运算
	memset(hist, 0, sizeof(hist));						// 初始化直方图的数组为0
	Histogram histogram(mouthImg, hist);					// 计算图片直方图
	// 计算最佳阈值
	pixelSum = mouthImg->width * mouthImg->height;

	OstuThreshold ostuThreshold(hist, pixelSum, 50);	//计算二值化的最佳阈值
	threshold = ostuThreshold.getostu();
	cvThreshold(mouthImg, mouthImg, threshold, 255, CV_THRESH_BINARY);// 对图像二值化
	// 显示二值化后的图像
	cvErode(mouthImg, mouthImg, NULL, 1);		//腐蚀图像  
	cvDilate(mouthImg, mouthImg, NULL, 1);	//膨胀图像
	cvShowImage("二值化后的嘴部", mouthImg);

	/*Mat Tempimg = cvarrToMat(mouthImg);
	Mat TempMouth = Tempimg.clone();
	vector<vector<Point>>contours;
	findContours(TempMouth, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);//寻找轮廓
	vector<vector<Point>>::iterator iter = contours.begin();
	while (iter != contours.end()) {
		if (iter->size() < 5) {
			iter = contours.erase(iter);
		}
		else {
			++iter;
		}
	}
	drawContours(TempMouth, contours, -1, Scalar(255), CV_FILLED);
	//cvShowImage("去除后的嘴部", mouthImg);
	imshow("去除后的嘴部", TempMouth);*/

	Point Center;
	int radius;
	Center.x = cvRound(MyMouTh.x+ faces[index].width * 0.15);
	Center.y = cvRound(MyMouTh.y+ faces[index].height * 0.05);
	radius = cvRound(MyMouTh.width + MyMouTh.height) * 0.22;
	//img = cvarrToMat(grayimg);
	circle(img, Center, radius, colors[6]);

	cvReleaseImage(&mouthImg);
	//cvWaitKey(0);
}

void Detect::detectEyeswithoutHaar(const int index)
{
	if (faces.size() > 0 && eyes.size() == 0) {//如果Haar检测到了脸但没有检测到眼睛
		Rect* largestFaceRect = nullptr;			// 存储检测到的最大的人脸矩形框
		float temp = 0;
		Point Center;
		int radius;
		IplImage* Srcimg;

		
		for (int i = 0; i < faces.size(); i++) {
			if ((faces.at(i).width * faces.at(i).height) > temp) {
				temp = faces.at(i).width * faces.at(i).height;
				largestFaceRect = new Rect(faces.at(i));
			}
		};
		temp = largestFaceRect->width / 8;
		// 根据人脸的先验知识分割出大致的人眼区域
		
		largestFaceRect->x = largestFaceRect->x + temp;
		largestFaceRect->width = largestFaceRect->width - 3 * temp / 2;
		largestFaceRect->height = largestFaceRect->height / 2;
		largestFaceRect->y = largestFaceRect->y + largestFaceRect->height / 2;
		largestFaceRect->height = largestFaceRect->height / 2;


		IplImage Tempimg = img;
		Srcimg = cvCloneImage(&Tempimg);
		cvSetImageROI(Srcimg, *largestFaceRect);				// 设置ROI为检测到的最大的人脸区域
		IplImage *faceImg = cvCreateImage(cvSize(largestFaceRect->width, largestFaceRect->height), IPL_DEPTH_8U, 1);
		//cvCopy(Srcimg, faceImg, NULL);
		faceImg = cvCloneImage(Srcimg);
		cvResetImageROI(Srcimg);								// 释放ROI
		//cvShowImage("分割后的人眼区域", faceImg);
		//aitKey(0);

		Rect eyeRectTemp = *largestFaceRect;
		// 根据人脸的先验知识分割出大致的左眼区域
		largestFaceRect->width /= 2;
		cvSetImageROI(Srcimg, *largestFaceRect);				// 设置ROI为检测到的最大的人脸区域
		IplImage *lEyeImg = cvCreateImage(cvSize(largestFaceRect->width, largestFaceRect->height), IPL_DEPTH_8U, 1);
		//cvCopy(Srcimg, lEyeImg, NULL);
		lEyeImg = cvCloneImage(Srcimg);
		cvResetImageROI(Srcimg);	// 释放ROI

		/********************标识****************************/
		Center.x = cvRound(eyeRectTemp.x + faces[index].width *0.15);
		Center.y = cvRound(eyeRectTemp.y + faces[index].height * 0.15);
		radius = cvRound(eyeRectTemp.width + eyeRectTemp.height) * 0.1;
		circle(img, Center, radius, colors[3]);
		/************************************************/

		//cvShowImage("大致的左眼区域", lEyeImg);
		//waitKey(0);

		// 根据人脸的先验知识分割出大致的右眼区域
		eyeRectTemp.x += eyeRectTemp.width / 2;
		eyeRectTemp.width /= 2;
		cvSetImageROI(Srcimg, eyeRectTemp);					// 设置ROI为检测到的最大的人脸区域
		IplImage *rEyeImg = cvCreateImage(cvSize(eyeRectTemp.width, eyeRectTemp.height), IPL_DEPTH_8U, 1);
		//cvCopy(Srcimg, rEyeImg, NULL);
		faceImg = cvCloneImage(Srcimg);
		rEyeImg = cvCloneImage(Srcimg);
		cvResetImageROI(Srcimg);								// 释放ROI

		/********************标识****************************/
		Center.x = cvRound(eyeRectTemp.x + faces[index].width * 0.15);
		Center.y = cvRound(eyeRectTemp.y + faces[index].height * 0.15);
		radius = cvRound(eyeRectTemp.width + eyeRectTemp.height) * 0.17;
		circle(img, Center, radius, colors[3]);
		/************************************************/
		//cvShowImage("大致的右眼区域", rEyeImg);
		//waitKey(0);

		cvReleaseImage(&Srcimg);
		cvReleaseImage(&faceImg);
		cvReleaseImage(&rEyeImg);
		cvReleaseImage(&lEyeImg);
		delete largestFaceRect;
	}
}

void Detect::showDetect()
{
	imshow("疲劳驾驶检测", img);
}
