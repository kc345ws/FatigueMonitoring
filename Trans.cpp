#include"Trans.h"

Trans::Trans(IplImage* srcImg, IplImage* dstImg, float a)
{
	int i, j;
	uchar* ptr = NULL;					// 指向图像当前行首地址的指针
	uchar* pixel = NULL;				// 指向像素点的指针
	float temp;

	dstImg = cvCreateImage(cvSize(srcImg->width, srcImg->height), IPL_DEPTH_8U, 1);
	cvCopy(srcImg, dstImg, NULL);
	int HEIGHT = dstImg->height;
	int WIDTH = dstImg->width;

	for (i = 0; i < HEIGHT; i++) {
		ptr = (uchar*)(srcImg->imageData + i * srcImg->widthStep);
		for (j = 0; j < WIDTH; j++) {
			pixel = ptr + j;

			// 非线性变换
			temp = *pixel + (a * (*pixel) * (255 - *pixel)) / 255;

			// 判断范围
			if (temp > 255)
				* pixel = 255;
			else if (temp < 0)
				* pixel = 0;
			else
				*pixel = (uchar)(temp + 0.5);// 四舍五入
		}
	}
}