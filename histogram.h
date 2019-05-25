/***************************************
功能：计算图像的直方图
输入：图像指针，存放直方图的数组首地址
****************************************/
#pragma once
#ifndef HISTOGRAM_H
#define HISTOGRAM_H
#include<opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp> 
#include<opencv2/highgui/highgui_c.h>
#include <opencv2/core/core.hpp>

class Histogram {
private:

public:
	Histogram(IplImage* img, int* hist);
};
#endif // !HISTOGRAM_H
