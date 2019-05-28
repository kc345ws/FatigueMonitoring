/************************************************************
功能：找出数列中限定区域内的最低点的位置
输入：
	int* project:  数列
	int size: 数列的长度
	int region: 中间区域的范围
输出：
	返回int结果位置, 错误时返回-1
************************************************************/
#pragma once
#ifndef EYEPOS_H
#define EYEPOS_H
#include<opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp> 
#include<opencv2/highgui/highgui_c.h>
#include <opencv2/core/core.hpp>
using namespace cv;
class EyePos {
private:
	//Point* point = new Point();
	typedef struct
	{
		int data;
		int index;
	}projectArr;

public:
	// qsort的函数参数
	int cmpInc(const void* a, const void* b){
		return (*(projectArr*)a).data - (*(projectArr*)b).data;
	}
	int getEyePos(int* project, int size, int region);
	int removeEyebrow(int* horiProject, int width, int height, int threshold);//去除眉毛
	void calEyeSocketRegion(Rect* eyeRect, int width, int height, int EyeCol, int EyeRow);
	void getEyeMinRect(Rect* eyeRect, int* horiProject, int* vertProject, int width, int height, int horiThreshold = 5, int vertThreshold = 3);
	double calMiddleAreaBlackPixRate(int* vertProject, Rect* eyeRect, int width, int height, int eyeCol, int MinEyeballBlackPixel);
	int getEyeState(double MinEyeballRectShape, double MinEyeballBlackPixelRate, double MinEyeballBeta); \
	int removeEyeglasses(int* horiProject, int width, int height, int threshold);//去除眼镜
};
#endif // !EYEPOS_H

