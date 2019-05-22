#include<iostream>
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp> 
#include<opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Scalar colors[] = {
	// ºì³È»ÆÂÌÇàÀ¶×Ï		
	CV_RGB(255, 0, 0),
	CV_RGB(255, 97, 0),
	CV_RGB(255, 255, 0),
	CV_RGB(0, 255, 0),
	CV_RGB(0, 255, 255),
	CV_RGB(0, 0, 255),
	CV_RGB(160, 32, 240)
};

int main() {
	
	VideoCapture cap = VideoCapture(0);
	cv::CascadeClassifier face_cascade;
	//face_cascade.load("D:\\Program Files\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalface_default.xml");
	face_cascade.load("Haar\\haarcascade_frontalface_default.xml");
	cv::CascadeClassifier eye_cascade; 
	//eye_cascade.load("D:\\Program Files\\opencv\\sources\\data\\haarcascades\\haarcascade_eye_tree_eyeglasses.xml");
	eye_cascade.load("Haar\\haarcascade_eye_tree_eyeglasses.xml");
	int times = 0;
	while (cap.isOpened()) {

		double rate = cap.get(cv::CAP_PROP_POS_FRAMES);
		Mat img;
		cap >> img;
		bool ret = cap.read(img);

		Mat gray;
		gray.create(img.size(), img.type());
		cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
		vector<Rect> faces;
		face_cascade.detectMultiScale(gray, faces, 1.3, 5,0);

		cout << "¼ì²âµ½ÈËÁ³¸öÊý:" << faces.size() << endl;
		
		for (int i = 0; i < faces.size(); i++) {
			Point Center;
			int radius;//°ë¾¶
			Center.x = cvRound(faces[i].x + faces[i].width * 0.5);
			Center.y = cvRound(faces[i].y + faces[i].height * 0.5);
			radius = cvRound((faces[i].width + faces[i].height) * 0.25);

			circle(img, Center, radius, colors[5]);

			Mat faceROI = gray(faces[i]);
			vector<Rect>eyes;
						
			eye_cascade.detectMultiScale(faceROI, eyes, 1.3, 3, 0);
			cout << "¼ì²âµ½ÑÛ¾¦¸öÊý:" << eyes.size() << endl;
			for (int j = 0; j < eyes.size(); j++) {
				Rect rect(faces[i].x + eyes[j].x, faces[i].y + eyes[j].y, eyes[j].width, eyes[j].height);
				rectangle(img, rect, colors[3], 2, 8, 0);
			}
		}
		imshow("ÊÓÆµ", img);
		if (cv::waitKey(1) && 1 == 2) {
			break;
		}
	}
	return 0;
}



