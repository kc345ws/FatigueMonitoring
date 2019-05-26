/*************************************************
功能：特定功能函数——根据眼睛闭合状态和是否检测到人脸
					  判断驾驶状态：正常？疲劳？
输入：
	int eyeCloseNum：检测过程中眼睛闭状态的总次数
	int maxEyeCloseDuration：检测过程中眼睛连续闭合的最大次数
	int failFaceNum：检测过程中未检测到人脸的总次数
	int maxFailFaceDuration：检测过程中连续未检测到人脸的最大次数
**************************************************/
#pragma once
#ifndef FATIGUE_H
#define FATIGUE_H



class Fatigue {
private:
	int eyeCloseValue;						// 眼睛闭合次数的权值
	int eyeCloseDurationValue;				// 眼睛连续闭合次数的权值
	int failFaceValue;						// 未检测到人脸的总次数的权值
	int failFaceDurationValue;				// 连续未检测到人脸的权值
	int compreValue;						// 综合权值
	int eyeCloseNumTab[32] = { 2,2,4,6,9,14,20,29,39,50,61,72,80,86,91,94,96,98,98,99,99,100,100,100,100,100,100,100,100,100, 100 ,100};
	//眼睛闭合次数权值分布
	int eyeCloseDurationTab[32] = { 2, 4, 9, 18, 32, 50, 68, 82, 91, 95, 98, 99, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,100 };
	//眼睛连续闭合次数权值分布
	int failFaceDurationTab[32] = { 2, 6, 14, 29, 50, 71, 86, 94, 98, 99, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,100 };
	//脸部连续监测失败次数权值分布
	//闭眼和未检测到脸部次数越多,综合权值越大，进入疲劳状态的可能性越大
public:
	Fatigue(int thresh, int eyeCloseNum, int maxEyeCloseDuration, int failFaceNum, int maxFailFaceDuration);
	int getisTired(int thresh);				//获取是否疲劳
};
#endif
