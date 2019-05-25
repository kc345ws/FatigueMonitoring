#include"detect.h"
#include<vector>

using namespace std;

Detect::Detect()
{
	cap = VideoCapture(0);
	face_cascade.load("Haar\\haarcascade_frontalface_default.xml");
	eye_cascade.load("Haar\\haarcascade_eye_tree_eyeglasses.xml");
}

void Detect::startDetect()
{
	if (checkisOpened()) {
		detectFace();
		showDetect();
	}
}

bool Detect::checkisOpened()
{
	if (cap.isOpened()) {
		return true;
	}
	return false;
}

void Detect::detectFace()
{
	//Mat img;
	cap >> img;
	//bool ret = cap.read(img);
	//Mat gray;
	gray.create(img.size(), img.type());
	cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
	//vector<Rect> faces;
	face_cascade.detectMultiScale(gray, faces, scale_factor, min_neighbors, flags);
	//cout << "¼ì²âµ½ÈËÁ³¸öÊý:" << faces.size() << endl;
	for (int i = 0; i < faces.size(); i++) {
		Point Center;
		int radius;//°ë¾¶
		Center.x = cvRound(faces[i].x + faces[i].width * 0.5);
		Center.y = cvRound(faces[i].y + faces[i].height * 0.5);
		radius = cvRound((faces[i].width + faces[i].height) * 0.25);

		circle(img, Center, radius, colors[5]);

		detectEyes(i);//¼ì²âÑÛ¾¦
	}
	/*imshow("Æ£ÀÍ¼ÝÊ»¼ì²â", img);
	if (cv::waitKey(1) && 1 == 2) {
		break;
	}*/	
}

void Detect::detectEyes(const int index)
{
	Mat faceROI = gray(faces[index]);
	//vector<Rect>eyes;
	eye_cascade.detectMultiScale(faceROI, eyes, 1.3, 3, 0);
	cout << "¼ì²âµ½ÑÛ¾¦¸öÊý:" << eyes.size() << endl;
	for (int j = 0; j < eyes.size(); j++) {
		//Rect rect(faces[i].x + eyes[j].x, faces[i].y + eyes[j].y, eyes[j].width, eyes[j].height);
		//rectangle(img, rect, colors[3], 2, 8, 0);
		Point Center;
		int radius;
		Center.x = cvRound(faces[index].x + eyes[j].x + eyes[j].width * 0.5);
		Center.y = cvRound(faces[index].y + eyes[j].y + eyes[j].height * 0.5);
		radius = cvRound(eyes[j].width + eyes[j].height) * 0.25;
		circle(img, Center, radius, colors[3]);
	}
}

void Detect::showDetect()
{
	imshow("Æ£ÀÍ¼ÝÊ»¼ì²â", img);
}
