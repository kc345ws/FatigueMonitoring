/********************************************************
功能：对图像进行线性点运算，实现图像增强
输入：
		IplImage* srcImg: 源灰度图像
		float a：乘系数a
输出：
		IplImage* dstImg：输出经过线性变换后的图像
********************************************************/
#pragma once
#ifndef NONLINETRANS_H
#define NONLINETRANS_H
#include<opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp> 
#include<opencv2/highgui/highgui_c.h>
#include <opencv2/core/core.hpp>

class Trans {
private:

public:
	Trans(IplImage* srcImg, IplImage* dstImg, float a);
};
#endif // !NONLINETRANS_H

