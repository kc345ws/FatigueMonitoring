#include"Fatigue.h"
#include<iostream>

using namespace std;
Fatigue::Fatigue(int thresh, int eyeCloseNum, int maxEyeCloseDuration, int failFaceNum, int maxFailFaceDuration) {
	// 查表计算四个指标的权值
	eyeCloseValue = eyeCloseNumTab[eyeCloseNum];
	eyeCloseDurationValue = eyeCloseDurationTab[maxEyeCloseDuration];
	failFaceValue = eyeCloseNumTab[failFaceNum];
	failFaceDurationValue = failFaceDurationTab[maxFailFaceDuration];
	// 综合权值
	compreValue = eyeCloseValue + eyeCloseDurationValue + failFaceValue + failFaceDurationValue;

	cout << "眼睛闭合次数的权值:" << eyeCloseValue << endl;
	cout << "眼睛连续闭合次数的权值:" << eyeCloseDurationValue << endl;
	cout << "未检测到人脸的总次数的权值:" << failFaceValue << endl;
	cout << "连续未检测到人脸的权值" << failFaceDurationValue << endl;
	cout << "综合权值" << compreValue << endl;
}

int Fatigue::getcompreValue()
{
	return compreValue;
}

