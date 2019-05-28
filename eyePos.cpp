#include"eyePos.h"
#include<iostream>
using namespace std;

int EyePos::getEyePos(int* project, int size, int region)
{
	projectArr* projectStruct = nullptr;
	projectArr* projectTemp = nullptr;
	int i, j, pos, sizeTemp, temp;

	// 分配projectStruct内存空间
	projectStruct = (projectArr*)malloc(size * sizeof(projectArr));
	projectTemp = (projectArr*)malloc(sizeof(projectArr));

	// 初始化内存空间
	for (i = 0; i < size; i++) {
		(projectStruct + i)->data = *(project + i);
		(projectStruct + i)->index = i;
	}

	// 对project从小到大快速排序
	for (i = 0; i <= size - 2; i++) {
		for (j = 0; j < size - i - 1; j++) {
			if ((projectStruct + j)->data > (projectStruct + j + 1)->data) {
				*projectTemp = *(projectStruct + j);
				*(projectStruct + j) = *(projectStruct + j + 1);
				*(projectStruct + j + 1) = *projectTemp;
			}
		}
	}

	// 寻找中间区域的最小值及其位置
	sizeTemp = size / 2;
	temp = 0;
	for (i = 0; i < size; i++) {
		temp = (projectStruct + i)->index;
		if ((temp > sizeTemp - region) && (temp < sizeTemp + region)) {
			pos = (projectStruct + i)->index;
			// 防止指针越界访问位置元素出现负数
			if (pos < 0)
				return -1;
			break;
		}
		else {
			// 防止整个数列不存在符合条件的元素
			if (i == size - 1)
				return -1;
		}
	}
	free(projectTemp);
	return pos;
}


/************************************************************
功能：搜索积分投影图中的最低点，从而消除眉毛的函数
输入：
	int* horiProject: 数列的指针
	int width:  数列的宽度
	int height: 数列的高度
	int threshold：分割眉毛的阈值，最多
输出：
	返回找到的最低点行位置，结果为int类型，即眉毛与眼睛的分割线
说明：
		1. 消除眉毛时可以调整eyeBrowThreshold来调整去除的效果
		2. 同时可以调整连续大于阈值的次数count来调整效果。
************************************************************/
int EyePos::removeEyebrow(int* horiProject, int width, int height, int threshold)
{
	int temp, temp1, count, flag, i;
	int eyeRow;
	int eyeBrowThreshold;

	// 定位人眼位置
	eyeBrowThreshold = (width - threshold) * 255;// 为了防止无法区分眼睛和眉毛的情况，可适当降低阈值

	// 消除眉毛区域
	temp = 100000000;
	temp1 = 0;
	count = 0;
	flag = 0;										// 表示当前搜索的位置在第一个最低谷之前
	eyeRow = 0;
	for (i = 0; i < height; i = i + 3) {
		count++;
		// 相当于递推滤波
		temp1 = *(horiProject + i) + *(horiProject + i + 1) + *(horiProject + i + 2);
		if ((temp1 < temp) & (flag == 0)) {
			temp = temp1;
			eyeRow = i;
			count = 0;
		}
		if (count >= 3 || i >= height - 2) {
			flag = 1;
			break;
		}
	}

	// 搜索第一个大于眼睛与眉毛分割阈值的点
	count = 0;
	for (i = eyeRow; i < height; i++) {
		if (*(horiProject + i) > eyeBrowThreshold) {
			eyeRow = i;
			count++;
			if (count >= 3) {				// count: 统计共有多少连续的行的投影值大于阈值；
				eyeRow = i;
				break;
			}
		}
		else
			count = 0;
	}

	// 防止没有眉毛错删眼睛的情况，根据实验结果调整参数
	if (eyeRow >= height / 2) {
		eyeRow = 0;
	}

	return eyeRow;
}


/************************************************************
功能：特定功能函数：根据人眼的中心大致计算眼眶的区域
输入：
	CvRect* eyeRect: 眼眶矩形区域的指针
	int width:  数列的宽度
	int height: 数列的高度
	int EyeCol：虹膜中心所在的列位置
	int EyeRow：虹膜中心所在的行位置
输出：
	以指针的方式返回眼眶的大致区域，eyeRect
************************************************************/
void EyePos::calEyeSocketRegion(Rect* eyeRect, int width, int height, int EyeCol, int EyeRow) {
	int temp, temp1;
	temp = EyeCol - width / 4;
	temp1 = EyeRow - height / 4;
	if ((temp < 0) && (temp1 < 0)) {
		eyeRect->x = 0;
		eyeRect->width = width / 2 + temp;
		eyeRect->y = 0;
		eyeRect->height = height / 2 + temp1;
	}
	else if ((temp < 0) && (temp1 > 0)) {
		eyeRect->x = 0;
		eyeRect->width = width / 2 + temp;
		eyeRect->y = temp1;
		eyeRect->height = height / 2;
	}
	else if ((temp > 0) && (temp1 < 0)) {
		eyeRect->x = temp;
		eyeRect->width = width / 2;
		eyeRect->y = 0;
		eyeRect->height = height / 2 + temp1;
	}
	else if ((temp > 0) && (temp1 > 0)) {
		eyeRect->x = temp;
		eyeRect->width = width / 2;
		eyeRect->y = temp1;
		eyeRect->height = height / 2;
	}
}



/************************************************************
功能：特定功能函数：计算人眼最小的矩形区域
输入：
	CvRect* eyeRect: 人眼最小的矩形区域的指针
	int* horiProject
	int* vertProject
	int width:  数列的宽度
	int height: 数列的高度
	int horiThreshold：水平方向的阈值
	int vertThreshold：垂直方向的阈值
输出：
	通过指针返回CvRect* eyeRect: 人眼最小的矩形区域的指针
************************************************************/
void EyePos::getEyeMinRect(Rect* eyeRect, int* horiProject, int* vertProject, int width, int height, int horiThreshold, int vertThreshold)
{
	int temp, temp1, i;

	temp1 = (width - horiThreshold) * 255;
	for (i = 0; i < height; i++) {
		if (*(horiProject + i) < temp1) {
			eyeRect->y = i;
			break;
		}
	}
	temp = i;				// 记录eyeRectTemp.y的位置
	//printf("eyeRectTemp->y: %d\n", eyeRect->y);

	if (temp != height) {
		// temp != HEIGHT: 防止没有符合*(subhoriProject + i) < temp1条件的位置；如果temp != HEIGHT则一定有满足条件的位置存在
		for (i = height - 1; i >= 0; i--) {
			if (*(horiProject + i) < temp1) {
				temp = i;
				break;
			}
		}
		if (temp == eyeRect->y)
			eyeRect->height = 1;
		else
			eyeRect->height = temp - eyeRect->y;
	}
	else {
		eyeRect->height = 1;
	}

	temp1 = (height - vertThreshold) * 255;
	for (i = 0; i < width; i++) {
		if (*(vertProject + i) < temp1) {
			eyeRect->x = i;
			break;
		}
	}
	temp = i;			// 记录eyeRectTemp.x的位置

	if (temp != width) {
		for (i = width - 1; i >= 0; i--) {
			if (*(vertProject + i) < temp1) {
				temp = i;
				break;
			}
		}
		// 防止宽度为0，显示图像时出错！
		if (temp == eyeRect->x)
			eyeRect->width = 1;
		else
			eyeRect->width = temp - eyeRect->x;
	}
	else {
		eyeRect->width = 1;
	}
}


/****************************** 判断眼睛状态 ************************************************
功能：统计lMinEyeballImg中的1/2区域内黑像素的比例
输入：
	int* vertProject：垂直投影结果
	CvRect* eyeRect：最小眼眶的区域
	int width：垂直投影数列的长度
	int eyeCol：最小眼眶中虹膜的中心所在的列数
	int MinEyeballBlackPixel：最小眼眶中的黑像素综总数
输出：
	double MinEyeballBeta：眼睛中心1/2区域黑色像素点占总黑像素点的比例
********************************************************************************************/
double EyePos::calMiddleAreaBlackPixRate(int* vertProject, Rect* eyeRect, int width, int height, int eyeCol, int MinEyeballBlackPixel) {
	int temp, temp1, count, i;
	double MinEyeballBeta;
	temp1 = 0;				// 中间1/2区域的白像素个数
	temp = 0;				// 中间1/2区域的黑像素个数
	count = 0;				// 临时变量
	MinEyeballBeta = 0;	// 初始化中间1/2区域的黑像素的比例
	if (width >= 8) {
		temp = eyeCol - eyeRect->x;// 最小眼眶的虹膜中心点所在的列
		// 以下if else 是保护性代码
		if (width / 2 > temp) {
			// 防止左边界越界的情况
			count = temp + width / 4;
			temp1 = temp - width / 4;
			if (temp1 < 0)
				i = 0;
			else
				i = temp1;
		}
		else {
			// 防止右边界越界的情况
			i = temp - width / 4;
			temp1 = temp + width / 4;
			if (temp1 < width)
				count = temp1;
			else
				count = width;
		}
		temp1 = 0;
		temp = 0;
		for (; i < count; i++)
			temp1 += *(vertProject + i);
		temp1 /= 255;
		temp = height * (width / 2) - temp1;								// 中间1/2区域的黑像素个数
		MinEyeballBeta = (double)temp / (double)MinEyeballBlackPixel;		// 中间1/2区域的黑像素占所有黑像素的比例
	}
	else {
		MinEyeballBeta = 0;
	}

	return MinEyeballBeta;
}



int EyePos::getEyeState(double MinEyeballRectShape, double MinEyeballBlackPixelRate, double MinEyeballBeta)
{
	//眼眶矩形的长宽比  最小眼眶黑色像素所占比例  1/2区域黑色像素所占比例
	int eyeState;//眼睛状态
	int funcResult;//模糊评价结果
	int shapeFuzzyLv, pixelFuzzyLv, betaFuzzyLv;	// 三个参数对应的模糊级别的值

	// 判定眼睛矩形区域的长宽比的模糊级别
	shapeFuzzyLv = 0;
	//如果长宽比过大 则视为眼睛已经闭上
	/*if ((MinEyeballRectShape >= 0) && (MinEyeballRectShape <= 0.8)) {
		shapeFuzzyLv = 0;
	}
	else if (MinEyeballRectShape <= 1.2) {
		shapeFuzzyLv = 2;
	}
	else if (MinEyeballRectShape <= 1.5) {
		shapeFuzzyLv = 6;
	}
	else if (MinEyeballRectShape <= 2.5) {
		shapeFuzzyLv = 8;
	}
	else if (MinEyeballRectShape <= 3) {
		shapeFuzzyLv = 6;
	}
	else {
		shapeFuzzyLv = 0;
	}*/

	if ((MinEyeballRectShape >= 0) && (MinEyeballRectShape <= 0.8)) {
		shapeFuzzyLv = 8;
	}
	else if (MinEyeballRectShape <= 1.2) {
		shapeFuzzyLv = 6;
	}
	else if (MinEyeballRectShape <= 1.5) {
		shapeFuzzyLv = 4;
	}
	else if (MinEyeballRectShape <= 2.5) {
		shapeFuzzyLv = 2;
	}
	else if (MinEyeballRectShape <= 3) {
		shapeFuzzyLv = 0;
	}
	else {
		shapeFuzzyLv = 0;
	}

	// 判定眼睛矩形区域黑像素点所占比例的模糊级别
	pixelFuzzyLv = 0;
	if ((MinEyeballBlackPixelRate >= 0) && (MinEyeballBlackPixelRate <= 0.4)) {
		pixelFuzzyLv = 0;
	}
	else if (MinEyeballBlackPixelRate <= 0.50) {
		pixelFuzzyLv = 2;
	}
	else if (MinEyeballBlackPixelRate <= 0.60) {
		pixelFuzzyLv = 6;
	}
	else if (MinEyeballBlackPixelRate <= 1) {
		pixelFuzzyLv = 8;
	}

	// 判定眼睛中心1/2区域黑色像素点占总黑像素点的比例的模糊级别
	betaFuzzyLv = 0;
	if ((MinEyeballBeta >= 0) && (MinEyeballBeta <= 0.3)) {
		betaFuzzyLv = 0;
	}
	else if (MinEyeballBeta <= 0.45) {
		betaFuzzyLv = 2;
	}
	else if (MinEyeballBeta <= 0.6) {
		betaFuzzyLv = 6;
	}
	else if (MinEyeballBeta <= 1) {
		betaFuzzyLv = 8;
	}

	// 模糊评价函数
	eyeState = 1;		// 默认是闭眼的
	funcResult = 2 * shapeFuzzyLv + 4 * pixelFuzzyLv + 4 * betaFuzzyLv;

	if (funcResult >= 42) {//如果模糊评价结果大于40则没闭眼
		eyeState = 0;
	}
	return eyeState;
}

int EyePos::removeEyeglasses(int* vertProject, int width, int height, int threshold)
{
	int temp, temp1, count, flag, i;
	int eyeRow;
	int eyeBrowThreshold;

	// 定位人眼位置
	eyeBrowThreshold = (width - threshold) * 255;// 为了防止无法区分眼睛和眼镜的情况，可适当降低阈值

	// 消除眼镜区域
	temp = 100000000;
	temp1 = 0;
	count = 0;
	flag = 0;										// 表示当前搜索的位置在第一个最低谷之前
	eyeRow = 0;
	for (i = 0; i < width; i = i + 3) {
		count++;
		// 相当于递推滤波
		temp1 = *(vertProject + i) + *(vertProject + i + 1) + *(vertProject + i + 2);
		if ((temp1 < temp) & (flag == 0)) {
			temp = temp1;
			eyeRow = i;
			count = 0;
		}
		if (count >= 3 || i >= width - 2) {
			flag = 1;
			break;
		}
	}

	// 搜索第一个大于眼睛与眼镜分割阈值的点
	count = 0;
	for (i = eyeRow; i < width; i++) {
		if (*(vertProject + i) > eyeBrowThreshold) {
			eyeRow = i;
			count++;
			if (count >= 3) {				// count: 统计共有多少连续的列的投影值大于阈值；
				eyeRow = i;
				break;
			}
		}
		else
			count = 0;
	}

	// 防止没有眉毛错删眼睛的情况，根据实验结果调整参数
	if (eyeRow >= width / 1.5) {
		eyeRow = 0;
	}

	return eyeRow;
}
