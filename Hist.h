/**************************************************
功能：计算图像直方图在水平方向和垂直方向的投影
输入：
	srcImg：源图像
输出：
	horiProj: 水平方向的投影结果；1 * height数组的指针，输入前记得初始化
	vertProj：垂直方向的投影结果；1 * width数组的指针，输入前记得初始化
**************************************************/
#pragma once
#ifndef HIST_H
#define HIST_H
#include<opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp> 
#include<opencv2/highgui/highgui_c.h>
#include <opencv2/core/core.hpp>

class Hist {
private:

public:
	Hist(){}
	Hist(IplImage* srcImg, int* horiProj, int* vertProj);
	void histProject(IplImage* srcImg, int* horiProj, int* vertProj);//计算图像直方图在水平方向和垂直方向的投影
};
#endif // !HIST_H
