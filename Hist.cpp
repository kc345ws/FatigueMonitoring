#include"Hist.h"

Hist::Hist(IplImage* srcImg, int* horiProj, int* vertProj)
{
	int i, j;
	uchar* ptr = NULL;					// 指向图像当前行首地址的指针
	uchar* temp = NULL;
	int HEIGHT = srcImg->height;
	int WIDTH = srcImg->width;


	for (i = 0; i < HEIGHT; i++) {
		ptr = (uchar*)(srcImg->imageData + i * srcImg->widthStep);
		for (j = 0; j < WIDTH; j++) {
			temp = ptr + j;				// 减少计算量
			*(horiProj + i) += *temp;	// 计算水平方向的投影
			*(vertProj + j) += *temp;	// 计算垂直方向的投影
		}
	}

}

void Hist::histProject(IplImage* srcImg, int* horiProj, int* vertProj) {
	int i, j;
	uchar* ptr = NULL;					// 指向图像当前行首地址的指针
	uchar* temp = NULL;
	int HEIGHT = srcImg->height;
	int WIDTH = srcImg->width;


	for (i = 0; i < HEIGHT; i++) {
		ptr = (uchar*)(srcImg->imageData + i * srcImg->widthStep);
		for (j = 0; j < WIDTH; j++) {
			temp = ptr + j;				// 减少计算量
			*(horiProj + i) += *temp;	// 计算水平方向的投影
			*(vertProj + j) += *temp;	// 计算垂直方向的投影
		}
	}
}
