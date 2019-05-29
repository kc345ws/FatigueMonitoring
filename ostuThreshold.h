/******************************************************
功能：用Ostu最大类间方差法计算二值化阈值
输入：
	hist：图像的直方图数组
	pixelSum：图像的像素总和
	CONST: 一个常数；为了适应各种特殊的要求，可实现在找到的最优分割阈值的基础上减去该常数
输出：
	threshold：最优阈值
******************************************************/
#pragma once
#ifndef ostuThreshold_h
#define ostuThreshold_h

class OstuThreshold {//最佳阈值处理
private:
	int threshold;
	int CON;
public:
	OstuThreshold(int* hist, int pixelSum, const int CONST);//计算二值化阈值
	int getostu() { return (threshold - CON); };
};
#endif // !ostuThreshold_h
