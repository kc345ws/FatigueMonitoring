#include"histogram.h"

Histogram::Histogram(IplImage* img, int* hist)
{
	int width = img->width;
	int height = img->height;
	int i, j;
	uchar* imageData = (uchar*)img->imageData;

	//统计灰度级包含的像素个数
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			hist[imageData[i * img->widthStep + j]]++;
		}
	}
}
