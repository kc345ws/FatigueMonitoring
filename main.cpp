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
#include"Hist.h"
#include"eyePos.h"

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
	int WIDTH = 0;									// 图像的宽度
	int HEIGHT = 0;									// 图像的高度
	int* horiProject = NULL;						// 水平方向的投影结果(数组指针)
	int* vertProject = NULL;						// 垂直方向的投影结果(数组指针)
	int* subhoriProject = NULL;						// 水平方向的投影结果(数组指针)
	int* subvertProject = NULL;						// 垂直方向的投影结果(数组指针)
	int rEyeCol = 0;								// 右眼所在的列数
	int lEyeCol = 0;								// 左眼所在的列数
	int lEyeRow = 0;								// 左眼所在的行数
	int rEyeRow = 0;								// 右眼所在的行数
	IplImage* lEyeImgNoEyebrow = NULL;				// 存储去除眉毛之后的左眼图像
	IplImage* rEyeImgNoEyebrow = NULL;				// 存储去除眉毛之后的右眼图像
	IplImage* lEyeballImg = NULL;					// 存储最终分割的左眼框的图像
	IplImage* rEyeballImg = NULL;					// 存储最终分割的右眼框的图像
	IplImage* lMinEyeballImg = NULL;				// 存储最终分割的最小的左眼框的图像
	IplImage* rMinEyeballImg = NULL;				// 存储最终分割的最小的右眼框的图像
	double lMinEyeballRectShape;					// 存储最小左眼眶的矩形长宽比值
	double rMinEyeballRectShape;					// 存储最小右眼眶的矩形长宽比值
	double lMinEyeballBeta;							// 存储最小左眼眶的中间1/2区域的黑像素比值
	double rMinEyeballBeta;							// 存储最小右边眼眶的中间1/2区域的黑像素比值
	int lMinEyeballBlackPixel;						// 存储最终分割的最小的左眼框的白色像素个数
	int rMinEyeballBlackPixel;						// 存储最终分割的最小的右眼框的白色像素个数
	double lMinEyeballBlackPixelRate;				// 存储最终分割的最小的左眼框的黑色像素占的比例
	double rMinEyeballBlackPixelRate;				// 存储最终分割的最小的右眼框的黑色像素占的比例
	float TIMETOTAL = 0;							// 时间总数
	int TempBow = 0;								// 临时低头次数
	int TempBlink = 0;								// 临时眨眼次数


	Detect *detect = new Detect();
	EyePos* eyepos = new EyePos();
	Fatigue* recoFatigueState = new Fatigue(FATIGUETHRESHOLD, eyeCloseNum, maxEyeCloseDuration, failFaceNum, maxFailFaceDuration);
	while (1) {
		for (globalK; globalK <= DETECTTIME; globalK++) {
			start = clock();
			TIMETOTAL = start / 10000;

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
				//int tempBow = detect->getBow();
				//tempBow++;
				//detect->setBow(tempBow);
				if (TempBow == 0) {
					TempBow++;//临时低头次数加1
				}

				cout << "没有获取到脸部" << endl;
				failFaceNum++;							// 统计未检测到人脸的次数
				failFaceDuration++;						// 统计连续未检测到人脸的次数
				// 检测过程中判断全是闭眼和检测不到人脸的情况，没有睁开眼的情况
				(eyeCloseDuration > maxEyeCloseDuration) ? maxEyeCloseDuration = eyeCloseDuration : maxEyeCloseDuration;
				eyeCloseDuration = 0;					// 统计一次检测过程中连续检测到闭眼状态的次数
				if (globalK == DETECTTIME) {
					// 当一次检测过程中，都没有检测到人脸，则要在此更新 maxFailFaceDuration 
					(failFaceDuration > maxFailFaceDuration) ? maxFailFaceDuration = failFaceDuration : maxFailFaceDuration;

					//cout << "达到疲劳状态" << endl;
					cout << "闭眼次数:" << eyeCloseNum << "最大连续闭眼次数:" << maxEyeCloseDuration << endl;
					cout << "获取脸部失败次数:" << failFaceNum << "最大连续获取脸部失败次数:" << maxFailFaceDuration << endl;

					// 进行疲劳状态的判别
					recoFatigueState = new Fatigue(FATIGUETHRESHOLD, eyeCloseNum, maxEyeCloseDuration, failFaceNum, maxFailFaceDuration);
					fatigueState = recoFatigueState->getisTired(FATIGUETHRESHOLD);
					if (fatigueState == 1) {
						cout << "驾驶员处于疲劳驾驶状态" << endl << endl;
						cout << "已进入疲劳状态无法统计眨眼和低头频率" << endl;
						//detect->setBlink(0);
						//detect->setBow(0);

						/*cout << "眨眼频率:" << detect->getBlinkper(TIMETOTAL) << "次/10秒" << endl;
						cout << "低头频率:" << detect->getBowper(TIMETOTAL) << "次/10秒" << endl;
						//TempBow++;
						detect->setBlink(0);
						detect->setBow(0);*/
					}
					else if (fatigueState == 0) {
						cout << "驾驶员处于正常驾驶状态" << endl << endl;
						cout << "眨眼频率:" << detect->getBlinkper(TIMETOTAL) << "次/10秒" << endl;
						cout << "低头频率:" << detect->getBowper(TIMETOTAL) << "次/10秒" << endl;
						//TempBow++;
						detect->setBlink(0);
						detect->setBow(0);
					}
					cvWaitKey(0);

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

				//cout << "低头频率:" << detect->getBowper(TIMETOTAL) << "次/10秒" << endl;
				//TempBow++;
				if (TempBow == 1) {//如果临时低头次数为1
					int Bow = detect->getBow();
					Bow++;
					detect->setBow(Bow);
					TempBow = 0;
				}

				// 统计连续未检测到人脸的次数中的最大数值
				(failFaceDuration > maxFailFaceDuration) ? maxFailFaceDuration = failFaceDuration : maxFailFaceDuration;
				failFaceDuration = 0;
				// 找到检测到的最大的人脸矩形区域
				temp = 0; //最大的人脸矩形区域面积
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
				//cvShowImage("分割后的人眼区域", faceImg);
				//aitKey(0);

				eyeRectTemp = *largestFaceRect;
				// 根据人脸的先验知识分割出大致的左眼区域
				largestFaceRect->width /= 2;
				cvSetImageROI(img, *largestFaceRect);				// 设置ROI为检测到的最大的人脸区域
				lEyeImg = cvCreateImage(cvSize(largestFaceRect->width, largestFaceRect->height), IPL_DEPTH_8U, 1);
				cvCopy(img, lEyeImg, NULL);
				cvResetImageROI(img);								// 释放ROI
				//cvShowImage("大致的左眼区域", lEyeImg);
				//waitKey(0);

				// 根据人脸的先验知识分割出大致的右眼区域
				eyeRectTemp.x += eyeRectTemp.width / 2;
				eyeRectTemp.width /= 2;
				cvSetImageROI(img, eyeRectTemp);					// 设置ROI为检测到的最大的人脸区域
				rEyeImg = cvCreateImage(cvSize(eyeRectTemp.width, eyeRectTemp.height), IPL_DEPTH_8U, 1);
				cvCopy(img, rEyeImg, NULL);
				cvResetImageROI(img);								// 释放ROI
				//cvShowImage("大致的右眼区域", rEyeImg);
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
				//cvShowImage("二值化后的左眼", lEyeImg);
				

				/*** 二值化右眼大致区域的图像 ***/
				cvSmooth(rEyeImg, rEyeImg, CV_MEDIAN);		// 中值滤波 默认窗口大小为3*3
				Trans RnonlineTrans(rEyeImg, rEyeImg, 0.8);	// 非线性点运算
				memset(hist, 0, sizeof(hist));				// 初始化直方图的数组为0
				Histogram Rhistogram(rEyeImg, hist);		// 计算图片直方图
				// 计算最佳阈值
				pixelSum = rEyeImg->width * rEyeImg->height;
				OstuThreshold RostuThreshold(hist, pixelSum, 45);
				cvThreshold(rEyeImg, rEyeImg, threshold, 255, CV_THRESH_BINARY);// 对图像二值化
				// 显示二值化后的图像
				//cvShowImage("二值化后的右眼", rEyeImg);
				//waitKey(0);
				/***************************************** 检测人眼 ********************************************/
				/** 如果有明显的眉毛区域，则分割去除眉毛 **/
				// 分割左眼眉毛
				HEIGHT = lEyeImg->height;
				WIDTH = lEyeImg->width;
				// 分配内存
				horiProject = (int*)malloc(HEIGHT * sizeof(int));
				vertProject = (int*)malloc(WIDTH * sizeof(int));
				if (horiProject == NULL || vertProject == NULL) {
					cout << "分配内存失败" << endl;
					cvWaitKey(0);
					return -1;
				}
				// 内存置零
				for (int i = 0; i < HEIGHT; i++) {
					*(horiProject + i) = 0;
				}
				for (int i = 0; i < WIDTH; i++) {
					*(vertProject + i) = 0;
				}

				Hist LhistProject;
				LhistProject.histProject(lEyeImg, horiProject, vertProject);
				//Hist LhistProject(lEyeImg, horiProject, vertProject);				// 计算直方图投影
				lEyeRow = eyepos->removeEyebrow(horiProject, WIDTH, HEIGHT, 10);	// 计算分割眉毛与眼框的位置

				// 分割右眼眉毛
				HEIGHT = rEyeImg->height;
				WIDTH = rEyeImg->width;
				// 分配内存
				horiProject = (int*)malloc(HEIGHT * sizeof(int));
				vertProject = (int*)malloc(WIDTH * sizeof(int));
				if (horiProject == NULL || vertProject == NULL) {
					cout << "分配内存失败" << endl;
					cvWaitKey(0);
					return -1;
				}
				// 内存置零
				for (int i = 0; i < HEIGHT; i++) {
					*(horiProject + i) = 0;
				}
				for (int i = 0; i < WIDTH; i++) {
					*(vertProject + i) = 0;
				}

				Hist RhistProject;
				RhistProject.histProject(rEyeImg, horiProject, vertProject);
				//Hist RhistProject(rEyeImg, horiProject, vertProject);							// 计算直方图投影
				rEyeRow = eyepos->removeEyebrow(horiProject, WIDTH, HEIGHT, 10);				// 计算分割眉毛与眼框的位置
				//eyepos->removeEyeglasses(vertProject, WIDTH, HEIGHT, 100);						// 去除眼镜
				
																								// 显示去除眉毛后的人眼大致区域
				eyeRect = cvRect(0, lEyeRow, lEyeImg->width, (lEyeImg->height - lEyeRow));		// 去眉毛的眼眶区域在lEyeImg中的矩形框区域
				cvSetImageROI(lEyeImg, eyeRect);												// 设置ROI为去除眉毛的眼眶，在下面释放ROI
				lEyeImgNoEyebrow = cvCreateImage(cvSize(eyeRect.width, eyeRect.height), IPL_DEPTH_8U, 1);
				cvCopy(lEyeImg, lEyeImgNoEyebrow, NULL);
				//cvShowImage("去除眉毛二值化左眼", lEyeImgNoEyebrow);

			/****************************************************************/
				//左眼去除镜框
				vector<vector<Point>> contour;

				Mat resulttemp;
				resulttemp = cvarrToMat(lEyeImgNoEyebrow);//浅拷贝
				Mat result = resulttemp.clone();//深拷贝
				Mat result1 = result.clone();
				
				findContours(result,contour,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);//寻找轮廓
				vector<vector<Point>>::iterator iter  = contour.begin();
				while (iter != contour.end()) {
					if (iter->size() < 30) {//删除轮廓面积小于30的
						iter = contour.erase(iter);
					}
					else {
						++iter;
					}
				}
				drawContours(result, contour, -1, Scalar(255), CV_FILLED);
				/*IplImage tempimg = result;
				IplImage* imgtmp = cvCloneImage(&tempimg);*/
				//cvErode(lEyeImgNoEyebrow, imgtmp, NULL, 2);		//腐蚀图像  
				//cvDilate(lEyeballImg, lEyeballImg, NULL, 2);	//膨胀图像
				
				Mat dst;
				//absdiff(result1, result, dst);
				//bitwise_not(result, dst);
				bitwise_xor(result1, result, dst);
				//imshow("result1", result1);
				//imshow("result", result);
				bitwise_not(dst,dst);
				IplImage LIMGTEMP = dst;
				lEyeImgNoEyebrow = cvCloneImage(&LIMGTEMP);
				//imshow("去除轮廓", dst);
				cvErode(lEyeImgNoEyebrow, lEyeImgNoEyebrow, NULL, 2);		//腐蚀图像  
				cvDilate(lEyeImgNoEyebrow, lEyeImgNoEyebrow, NULL, 2);	//膨胀图像
				cvDilate(lEyeImgNoEyebrow, lEyeImgNoEyebrow, NULL, 1);	//膨胀图像
				cvShowImage("L去除镜框", lEyeImgNoEyebrow);

				/****************************************************************/
				//右眼去除眉毛
				//cvWaitKey(0);


				eyeRectTemp = cvRect(0, rEyeRow, rEyeImg->width, (rEyeImg->height - rEyeRow));	// 去眉毛的眼眶区域在rEyeImg中的矩形框区域
				cvSetImageROI(rEyeImg, eyeRectTemp);											// 设置ROI为去除眉毛的眼眶，在下面释放ROI
				rEyeImgNoEyebrow = cvCreateImage(cvSize(eyeRectTemp.width, eyeRectTemp.height), IPL_DEPTH_8U, 1);
				cvCopy(rEyeImg, rEyeImgNoEyebrow, NULL);
				//cvShowImage("去除眉毛二值化右眼", rEyeImgNoEyebrow);
				//cvWaitKey(0);
				/****************************************************************/
				//右眼去除镜框
				vector<vector<Point>> Rcontour;

				Mat Rresulttemp;
				Rresulttemp = cvarrToMat(rEyeImgNoEyebrow);//浅拷贝
				Mat Rresult = Rresulttemp.clone();//深拷贝
				Mat Rresult1 = Rresult.clone();

				findContours(Rresult,Rcontour, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);//寻找轮廓
				vector<vector<Point>>::iterator Riter = Rcontour.begin();
				while (Riter != Rcontour.end()) {
					if (Riter->size() < 20) {//删除轮廓面积小于30的
						Riter = Rcontour.erase(Riter);
					}
					else {
						++Riter;
					}
				}
				drawContours(Rresult, Rcontour, -1, Scalar(255), CV_FILLED);
				/*IplImage tempimg = result;
				IplImage* imgtmp = cvCloneImage(&tempimg);*/
				//cvErode(lEyeImgNoEyebrow, imgtmp, NULL, 2);		//腐蚀图像  
				//cvDilate(lEyeballImg, lEyeballImg, NULL, 2);	//膨胀图像

				Mat Rdst;
				//absdiff(result1, result, dst);
				//bitwise_not(result, dst);
				bitwise_xor(Rresult1, Rresult, Rdst);
				//imshow("result1", Rresult1);
				//imshow("result", Rresult);
				bitwise_not(Rdst, Rdst);
				IplImage RLIMGTEMP = Rdst;
				rEyeImgNoEyebrow = cvCloneImage(&RLIMGTEMP);
				//imshow("去除轮廓", Rdst);
				cvErode(rEyeImgNoEyebrow, rEyeImgNoEyebrow, NULL, 2);		//腐蚀图像  
				cvDilate(rEyeImgNoEyebrow, rEyeImgNoEyebrow, NULL, 2);	//膨胀图像
				cvDilate(rEyeImgNoEyebrow, rEyeImgNoEyebrow, NULL, 1);	//膨胀图像
				cvShowImage("r去除镜框", rEyeImgNoEyebrow);
				cvWaitKey(100);

				/****************************************************************/

				///////////////// 定位眼睛中心点在去除眉毛图像中的行列位置 ///////////////////
				HEIGHT = lEyeImgNoEyebrow->height;
				WIDTH = lEyeImgNoEyebrow->width;
				// 分配内存
				subhoriProject = (int*)malloc(HEIGHT * sizeof(int));
				subvertProject = (int*)malloc(WIDTH * sizeof(int));
				if (subhoriProject == NULL || subvertProject == NULL) {
					cout << "分配内存失败" << endl;
					cvWaitKey(0);
					return -1;
				}
				// 内存置零
				for (int i = 0; i < HEIGHT; i++) {
					*(subhoriProject + i) = 0;
				}
				for (int i = 0; i < WIDTH; i++) {
					*(subvertProject + i) = 0;
				}

				RhistProject.histProject(lEyeImgNoEyebrow, subhoriProject, subvertProject);
				//Hist RhistProject1(lEyeImgNoEyebrow, subhoriProject, subvertProject);	// 重新对分割出的左眼图像进行积分投影
				lEyeRow = eyepos->getEyePos(subhoriProject, HEIGHT, HEIGHT / 5);	// 定位左眼所在的行
				lEyeCol = eyepos->getEyePos(subvertProject, WIDTH, WIDTH / 5);	// 定位左眼所在的列


				HEIGHT = rEyeImgNoEyebrow->height;
				WIDTH = rEyeImgNoEyebrow->width;
				// 分配内存
				subhoriProject = (int*)malloc(HEIGHT * sizeof(int));
				subvertProject = (int*)malloc(WIDTH * sizeof(int));
				if (subhoriProject == NULL || subvertProject == NULL) {
					cout << "分配内存失败" << endl;
					cvWaitKey(0);
					return -1;
				}
				// 内存置零
				for (int i = 0; i < HEIGHT; i++) {
					*(subhoriProject + i) = 0;
				}
				for (int i = 0; i < WIDTH; i++) {
					*(subvertProject + i) = 0;
				}
				RhistProject.histProject(rEyeImgNoEyebrow, subhoriProject, subvertProject);
				//RhistProject(rEyeImgNoEyebrow, subhoriProject, subvertProject);	// 重新对分割出的右眼图像进行积分投影
				rEyeRow = eyepos->getEyePos(subhoriProject, HEIGHT, HEIGHT / 5);	// 定位右眼所在的行
				rEyeCol = eyepos->getEyePos(subvertProject, WIDTH, WIDTH / 5);	// 定位右眼所在的列
				
				// 标记眼睛的位置
				cvCircle(lEyeImgNoEyebrow, cvPoint(lEyeCol, lEyeRow), 3, CV_RGB(0, 0, 255), 1, 8, 0);
				cvCircle(rEyeImgNoEyebrow, cvPoint(rEyeCol, rEyeRow), 3, CV_RGB(0, 0, 255), 1, 8, 0);
				//cvShowImage("左眼中心", lEyeImgNoEyebrow);
				//cvShowImage("右眼中心", rEyeImgNoEyebrow);
				//cvWaitKey(0);

				/********************************** 判断人眼睁闭状态 ***********************************/

			////////////////// 分割出以找到的中心为中心的大致眼眶 /////////////////
			// 左眼眶
				HEIGHT = lEyeImgNoEyebrow->height;
				WIDTH = lEyeImgNoEyebrow->width;
				// 计算大致眼眶的区域: eyeRect
				eyeRect = cvRect(0, 0, WIDTH, HEIGHT);
				//Rect* tempRect = new Rect(0, 0, HEIGHT, WIDTH);
				eyepos->calEyeSocketRegion(&eyeRect, WIDTH, HEIGHT, lEyeCol, lEyeRow);
				
				cvSetImageROI(lEyeImgNoEyebrow, eyeRect);		// 设置ROI为检测到眼眶区域
				
				//Mat MatlEyeballImg;
				lEyeballImg = cvCreateImage(cvSize(lEyeImgNoEyebrow->width, lEyeImgNoEyebrow->height), IPL_DEPTH_8U, 1);
				//cvCopy(lEyeImgNoEyebrow, lEyeballImg,NULL);
				lEyeballImg = cvCloneImage(lEyeImgNoEyebrow);
				cvResetImageROI(lEyeImgNoEyebrow);// 释放ROI
				//cvShowImage("左眼眶", lEyeballImg);

				// 右眼眶
				HEIGHT = rEyeImgNoEyebrow->height;
				WIDTH = rEyeImgNoEyebrow->width;
				// 计算大致眼眶的区域: eyeRectTemp
				eyeRect = cvRect(0, 0, WIDTH, HEIGHT);
				eyepos->calEyeSocketRegion(&eyeRect, WIDTH, HEIGHT, rEyeCol, rEyeRow);
				
				
				cvSetImageROI(rEyeImgNoEyebrow, eyeRect);		// 设置ROI为检测到眼眶区域
				rEyeballImg = cvCreateImage(cvSize(rEyeImgNoEyebrow->width, rEyeImgNoEyebrow->height), IPL_DEPTH_8U, 1);
				//cvCopy(rEyeImgNoEyebrow, rEyeballImg, NULL);
				rEyeballImg = cvCloneImage(rEyeImgNoEyebrow);
				cvResetImageROI(rEyeImgNoEyebrow);
				//cvShowImage("右眼眶", rEyeballImg);
				//cvWaitKey(0);
				/////////////////////////// 闭运算 ///////////////////////////
				cvErode(lEyeballImg, lEyeballImg, NULL, 2);		//腐蚀图像  
				cvDilate(lEyeballImg, lEyeballImg, NULL, 2);	//膨胀图像
				//cvShowImage("左眼球", lEyeballImg);

				cvErode(rEyeballImg, rEyeballImg, NULL, 1);		//腐蚀图像  
				//先腐蚀图像去除噪点
				cvDilate(rEyeballImg, rEyeballImg, NULL, 1);	//膨胀图像
				//再膨胀图像使眼球图像变大
				//cvShowImage("右眼球", rEyeballImg);
				//cvWaitKey(0);

				/////////////////// 计算最小眼睛的矩形区域 ////////////////////

			///////////////////////////左眼
				HEIGHT = lEyeballImg->height;
				WIDTH = lEyeballImg->width;

				// 分配内存
				subhoriProject = (int*)malloc(HEIGHT * sizeof(int));
				subvertProject = (int*)malloc(WIDTH * sizeof(int));
				if (subhoriProject == NULL || subvertProject == NULL) {
					cout << "分配内存失败" << endl;
					cvWaitKey(0);
					return -1;
				}
				// 内存置零
				for (int i = 0; i < HEIGHT; i++) {
					*(subhoriProject + i) = 0;
				}
				for (int i = 0; i < WIDTH; i++) {
					*(subvertProject + i) = 0;
				}
				//histProject(lEyeballImg, subhoriProject, subvertProject);
				LhistProject.histProject(lEyeballImg, subhoriProject, subvertProject);
				// 计算左眼最小的矩形区域
				eyeRectTemp = cvRect(0, 0, 1, 1);		// 初始化
				eyepos->getEyeMinRect(&eyeRectTemp, subhoriProject, subvertProject, WIDTH, HEIGHT, 5, 3);
	
				// 计算最小左眼矩形的长宽比,  判断眼睛状态时用的到
				lMinEyeballRectShape = (double)eyeRectTemp.width / (double)eyeRectTemp.height;
				//printf("\nlMinEyeballRectShape: %f\n", lMinEyeballRectShape);

				cvSetImageROI(lEyeballImg, eyeRectTemp);		// 设置ROI为检测到最小面积的眼眶
				lMinEyeballImg = cvCreateImage(cvSize(lEyeballImg->width, lEyeballImg->height), IPL_DEPTH_8U, 1);
				//cvCopy(lEyeballImg, lMinEyeballImg, NULL);
				lMinEyeballImg = cvCloneImage(lEyeballImg);
				cvResetImageROI(lEyeballImg);
				//cvShowImage("左眼眶最小矩形区域", lMinEyeballImg);

				
				//cvShowImage("右眼球", rEyeballImg);

				//cvWaitKey(0);
				

				////////////////////////  统计左眼黑像素个数  /////////////////////
				HEIGHT = lMinEyeballImg->height;
				WIDTH = lMinEyeballImg->width;

				// 分配内存
				subhoriProject = (int*)malloc(HEIGHT * sizeof(int));
				subvertProject = (int*)malloc(WIDTH * sizeof(int));
				if (subhoriProject == NULL || subvertProject == NULL) {
					cout << "分配内存失败" << endl;
					cvWaitKey(0);
					return -1;
				}
				// 内存置零
				for (int i = 0; i < HEIGHT; i++) {
					*(subhoriProject + i) = 0;
				}
				for (int i = 0; i < WIDTH; i++) {
					*(subvertProject + i) = 0;
				}

				//histProject(lMinEyeballImg, subhoriProject, subvertProject);
				LhistProject.histProject(lMinEyeballImg, subhoriProject, subvertProject);
				// 统计lEyeballImg中黑色像素的个数
				temp = 0;	// 白像素个数
				for (int i = 0; i < WIDTH; i++) {
					temp += *(subvertProject + i);
				}
				temp /= 255;
				lMinEyeballBlackPixel = WIDTH * HEIGHT - temp;
				lMinEyeballBlackPixelRate = (double)lMinEyeballBlackPixel / (double)(WIDTH * HEIGHT);

				// 统计lMinEyeballImg中的1/2区域内黑像素的比例
				lMinEyeballBeta = 0;
				lMinEyeballBeta = eyepos->calMiddleAreaBlackPixRate(subvertProject, &eyeRectTemp, WIDTH, HEIGHT, lEyeCol, lMinEyeballBlackPixel);

				

				////////////////////////////////////右眼
				HEIGHT = rEyeballImg->height;
				WIDTH = rEyeballImg->width;
				// 分配内存
				subhoriProject = (int*)malloc(HEIGHT * sizeof(int));
				subvertProject = (int*)malloc(WIDTH * sizeof(int));
				if (subhoriProject == NULL || subvertProject == NULL) {
					cout << "分配内存失败" << endl;
					cvWaitKey(0);
					return -1;
				}
				// 内存置零
				for (int i = 0; i < HEIGHT; i++) {
					*(subhoriProject + i) = 0;
				}
				for (int i = 0; i < WIDTH; i++) {
					*(subvertProject + i) = 0;
				}
				//histProject(rEyeballImg, subhoriProject, subvertProject);
				RhistProject.histProject(rEyeballImg, subhoriProject, subvertProject);
				// 计算右眼最小的矩形区域
				eyeRectTemp = cvRect(0, 0, 1, 1);
				eyepos->getEyeMinRect(&eyeRectTemp, subhoriProject, subvertProject, WIDTH, HEIGHT, 5, 3);

				// 计算最小右眼矩形的长宽比，判断眼睛状态时用的到
				rMinEyeballRectShape = (double)eyeRectTemp.width / (double)eyeRectTemp.height;

				cvSetImageROI(rEyeballImg, eyeRectTemp);		// 设置ROI为检测到最小面积的眼眶
				rMinEyeballImg = cvCreateImage(cvSize(rEyeballImg->width, rEyeballImg->height), IPL_DEPTH_8U, 1);
				//cvCopy(rEyeballImg, rMinEyeballImg, NULL);
				rMinEyeballImg = cvCloneImage(rEyeballImg);
				cvResetImageROI(rEyeballImg);
				//cvShowImage("右眼眶最小矩形区域", rMinEyeballImg);

				//cvWaitKey(0);
				////////////////////////  统计右眼黑像素个数  /////////////////////
				HEIGHT = rMinEyeballImg->height;
				WIDTH = rMinEyeballImg->width;

				// 分配内存
				subhoriProject = (int*)malloc(HEIGHT * sizeof(int));
				subvertProject = (int*)malloc(WIDTH * sizeof(int));
				if (subhoriProject == NULL || subvertProject == NULL) {
					cout << "分配内存失败" << endl;
					cvWaitKey(0);
					return -1;
				}
				// 内存置零
				for (int i = 0; i < HEIGHT; i++) {
					*(subhoriProject + i) = 0;
				}
				for (int i = 0; i < WIDTH; i++) {
					*(subvertProject + i) = 0;
				}
				//histProject(rMinEyeballImg, subhoriProject, subvertProject);// 计算直方图积分投影
				RhistProject.histProject(rMinEyeballImg, subhoriProject, subvertProject);
				// 统计REyeballImg中黑色像素的个数
				temp = 0;
				for (int i = 0; i < WIDTH; i++) {
					temp += *(subvertProject + i);
				}
				temp /= 255;
				rMinEyeballBlackPixel = WIDTH * HEIGHT - temp;
				rMinEyeballBlackPixelRate = (double)rMinEyeballBlackPixel / (double)(WIDTH * HEIGHT);

				// 统计lMinEyeballImg中的1/2区域内黑像素的比例
				rMinEyeballBeta = 0;
				rMinEyeballBeta = eyepos->calMiddleAreaBlackPixRate(subvertProject, &eyeRectTemp, WIDTH, HEIGHT, rEyeCol, rMinEyeballBlackPixel);

				// 判断眼睛睁闭情况
				lEyeState = 1;		// 左眼状态，默认闭眼
				rEyeState = 1;		// 右眼状态，默认闭眼
				eyeState = 1;		// 眼睛综合状态，默认闭眼
				//cvWaitKey(0);
				if (lMinEyeballBlackPixel > 100) {
					//lEyeState = eyepos->getEyeState(lMinEyeballRectShape, lMinEyeballBlackPixelRate, lMinEyeballBeta);
					lEyeState = 0;
				}
				else {//最小黑色像素总量小于50则直接视为闭眼了
					lEyeState = 1;
				}

				if (detect->getEyes().size() > 0) {
					lEyeState = 0;//如果Haar直接识别到了眼睛
				}


				if (rMinEyeballBlackPixel > 100) {
					//rEyeState = eyepos->getEyeState(rMinEyeballRectShape, rMinEyeballBlackPixelRate, rMinEyeballBeta);
					rEyeState = 0;
				}
				else {
					rEyeState = 1;
				}

				if (detect->getEyes().size() > 0) {
						rEyeState = 0;//如果Haar直接识别到了眼睛
				}

				(lEyeState + rEyeState) == 2 ? eyeState = 1 : eyeState = 0;//左右眼都闭合才算闭合

				// 统计眼睛闭合的次数
				if (eyeState == 1) {//如果眼睛闭合
					eyeCloseNum++;					// 统计 eyeCloseNum 眼睛闭合次数
					eyeCloseDuration++;

					/*if (TempBlink == 1) {
						int tempBlink = detect->getBlink();
						tempBlink++;
						detect->setBlink(tempBlink);//增加眨眼次数
					}*/
					if (TempBlink == 0) {
						TempBlink++;
					}
					

					if (globalK == DETECTTIME) {
						// 检测过程中判断全是闭眼情况，没有睁眼和检测不到人脸的情况
						(eyeCloseDuration > maxEyeCloseDuration) ? maxEyeCloseDuration = eyeCloseDuration : maxEyeCloseDuration;
						eyeCloseDuration = 0;
					}
				}
				else {
					(eyeCloseDuration > maxEyeCloseDuration) ? maxEyeCloseDuration = eyeCloseDuration : maxEyeCloseDuration;
					eyeCloseDuration = 0;

					if (TempBlink == 1) {
						int tempBlink = detect->getBlink();
						tempBlink++;
						detect->setBlink(tempBlink);//增加眨眼次数
						TempBlink = 0;
					}
				}
		} // 承接判断是否检测到人脸的if语句

		// 计时：执行一次循环的时间
		stop = clock();
		if (eyeState == 0) {
			cout << "眼睛状态:睁"  << endl;
		}
		else {
			cout << "眼睛状态:闭" << endl;
		}

		// 调整循环变量，进入下一次检测过程
		if (globalK == DETECTTIME) {
			//printf("eyeCloseNum: %d\tmaxEyeCloseDuration: %d\n", eyeCloseNum, maxEyeCloseDuration);
			//printf("failFaceNum: %d\tmaxFailFaceDuration: %d\n", failFaceNum, maxFailFaceDuration);

			// 进行疲劳状态的判别
			//Fatigue isFatigue(FATIGUETHRESHOLD, eyeCloseNum, maxEyeCloseDuration, failFaceNum, maxFailFaceDuration);
			//fatigueState = recoFatigueState(FATIGUETHRESHOLD, eyeCloseNum, maxEyeCloseDuration, failFaceNum, maxFailFaceDuration);

			recoFatigueState = new Fatigue(FATIGUETHRESHOLD, eyeCloseNum, maxEyeCloseDuration, failFaceNum, maxFailFaceDuration);
			fatigueState = recoFatigueState->getisTired(FATIGUETHRESHOLD);
			if (fatigueState == 1) {
				cout << "驾驶员处于疲劳驾驶状态" << endl << endl;
				cout << "已进入疲劳状态无法统计眨眼和低头频率" << endl;
				//detect->setBlink(0);
				//detect->setBow(0);

				//cout << "眨眼频率:" << detect->getBlinkper(TIMETOTAL) << "次/10秒" << endl;
				//cout << "低头频率:" << detect->getBowper(TIMETOTAL) << "次/10秒" << endl;
				//TempBow++;
				//detect->setBlink(0);
				//detect->setBow(0);
			}
			else if (fatigueState == 0) {
				cout << "驾驶员处于正常驾驶状态" << endl << endl;
				cout << "眨眼频率:" << detect->getBlinkper(TIMETOTAL) << "次/10秒" << endl;
				cout << "低头频率:" << detect->getBowper(TIMETOTAL) << "次/10秒" << endl;
				detect->setBlink(0);
				detect->setBow(0);
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
			char c = cvWaitKey(0);
			if (c == 27)
				break;
			else
				continue;
			}
		}
	}
	return 0;
}

