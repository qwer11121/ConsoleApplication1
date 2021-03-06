// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>

#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 300

using namespace cv;
using namespace std;

// 锐化图像，使边缘更突出
// 算子为 
//        0 -1  0
//       -1  5 -1
//        0 -1  0 
void sharp(const Mat &image, Mat& result, int repeat=1)
{
	int i, j;
	result.create(image.size(), image.type());

	int nc = image.cols*image.channels();
	int ch = image.channels();
	//ch = 1;

	for (i = 1; i < image.rows - 1; i++)
	{
		// uchar  0-255
		const uchar* up_line = image.ptr<uchar>(i - 1);
		const uchar* mid_line = image.ptr<uchar>(i);
		const uchar* down_line = image.ptr<uchar>(i + 1);
		uchar* cur_line = result.ptr<uchar>(i);

		for (int r = 0; r < repeat; r++)
		{
			for (j = ch; j < nc - ch; j++)
			{
				cur_line[j] = saturate_cast<uchar>(5 * mid_line[j] - mid_line[j - ch] - mid_line[j + ch] - up_line[j] - down_line[j]);
			}
		}

	}

	// 边缘置 0
	result.row(0).setTo(Scalar(0));
	result.row(result.rows - 1).setTo(Scalar(0));
	result.col(0).setTo(Scalar(0));
	result.col(result.cols - 1).setTo(Scalar(0));
}

// 魔杖效果，被选中的涂白
class colorDetect
{
private:
	int minDist;
	Vec3b target;
	Mat result;

public:
	//colorDetect();

	Mat process(const Mat& image)
	{
		// 复制原图并转为Lab格式
		Mat img = image.clone();
		result.create(image.rows, image.cols, CV_8U);
		cvtColor(image, img, CV_BGR2Lab);

		// 通过临时Mat对象将target从BGR格式转为Lab格式
		Mat temp(1, 1, CV_8UC3);
		temp.at<Vec3b>(0, 0) = target;
		cvtColor(temp, temp, CV_BGR2Lab);
		target = temp.at<Vec3b>(0, 0);

		Mat_<Vec3b>::iterator it = img.begin<Vec3b>();
		Mat_<Vec3b>::iterator itend = img.end<Vec3b>();
		Mat_<uchar>::iterator itout = result.begin<uchar>();
		while (it != itend)
		{
			int dist = static_cast<int>(norm<int, 3>(Vec3i((*it)[0] - target[0], (*it)[1] - target[1], (*it)[2] - target[2])));
			if (dist < minDist)
				(*itout) = 255;    // 涂白
			else
				(*itout) = 0;
			it++;
			itout++;
		}
		return result;
	}

	void SetTargetColor(uchar red, uchar green, uchar blue)
	{
		target[0] = blue;
		target[1] = green;
		target[2] = red;
	}

	void SetTargetColor(Vec3b color)
	{
		target = color;
	}

	void SetMinDistance(int dist)
	{
		minDist = dist;
	}
};

// 显示单通道直方图
Mat getHistImg(const MatND& hist)
{
	double maxVal = 0;
	double minVal = 0;

	//找到直方图中的最大值和最小值
	minMaxLoc(hist, &minVal, &maxVal, 0, 0);
	int histSize = hist.rows;
	Mat histImg(histSize, histSize, CV_8U, Scalar(255));
	// 设置最大峰值为图像高度的90%
	int hpt = static_cast<int>(0.9*histSize);

	for (int h = 0; h<histSize; h++)
	{
		float binVal = hist.at<float>(h);
		int intensity = static_cast<int>(binVal*hpt / maxVal);
		line(histImg, Point(h, histSize), Point(h, histSize - intensity), Scalar::all(0));
	}

	return histImg;
}


MatND getOneChannelHist(const Mat& img, int channel)
{
	MatND hist;
	const int channels[1] = { channel };
	const int histSize[1] = { 256 };
	float hranges[2] = { 0,255 };
	const float* ranges[1] = { hranges };
	calcHist(&img, 1, channels, Mat(), hist, 1, histSize, ranges, true, false);
	return hist;
}

MatND getHist(const Mat& img)
{
	MatND hist;
	const int channels[3] = { 0,1,2 };
	const int histSize[3] = { 256,256,256 };
	float hrange[2] = { 0,255 };
	const float* ranges[3] = { hrange,hrange,hrange };
	calcHist(&img, 1, channels, Mat(), hist, 3, histSize, ranges);
	return hist;
}

void changeBGRLightness(const Mat& img, Mat& output, int offset)
{
	Mat temp;
	cvtColor(img, temp, CV_BGR2Lab);


	int nr = temp.rows;
	int nc = temp.cols;
	if (temp.isContinuous())
	{
		nc = nr*nc;
		nr = 1;
	}
	int i, j;
	for (i = 0; i < nr; i++)
	{
		Vec3b* p = temp.ptr<Vec3b>(i);
		for (j = 0; j < nc; j++)
		{
			p[j][0] = saturate_cast<uchar>(p[j][0] + offset);
		}
	}
	cvtColor(temp, output, CV_Lab2BGR);
}

void changeBGRLightness2(Mat& img, Mat& output, int offset)
{
	//output.create(img.size(), img.type());
	output = img.clone();
	int nr = output.rows;
	int nc = output.cols;
	if ( output.isContinuous())
	{
		nc = nr*nc*output.channels();
		nr = 1;
	}
	int i, j;
	for (i = 0; i < nr; i++)
	{
		uchar* p = output.ptr<uchar>(i);
		for (j = 0; j < nc; j++)
		{
			*p = saturate_cast<uchar>(*p + offset);
			++p;
		}
	}
}


void showImage(const Mat& image, const string title = "image", const int width = 0, const int height = 0)
{	
	if (width != 0 && height != 0)
	{
		namedWindow(title, WINDOW_NORMAL);
		resizeWindow(title, width, height);
	}
	imshow(title, image);
}

// return full path of a image in %Pictures% folder
char* getImagePath(const char* name)
{
	char* imgPath;
	size_t envCount;
	_dupenv_s(&imgPath, &envCount, "Pictures");
	if (envCount > 0)
	{
		int new_length = envCount + strlen(name) + 1;
		char* fullPath = new char[new_length];
		strcpy_s(fullPath, new_length, imgPath);
		strcat_s(fullPath, new_length, "\\");    //  "\\" is one charactor in memory
		strcat_s(fullPath, new_length, name);
		return fullPath;
	}
	else
	{
		return "";
	}
}

int main()
{
	Mat img1, img2, img3;
	//img1 = imread(getImagePath("32.png"));
	//img2 = imread(getImagePath("screenshot.png"));
	//img1 = imread(getImagePath("cat.jpg"));

	VideoCapture cap;
	namedWindow("Cam", WINDOW_NORMAL);
	namedWindow("output", WINDOW_NORMAL);
	resizeWindow("Cam", 320, 240);
	resizeWindow("output", 320, 240);
	cap.open(0);

	while (true)
	{
		cap.read(img1);
		changeBGRLightness2(img1, img2, 70);

		imshow("Cam", img1);
		imshow("output", img2);
		waitKey(1);
	}


	//showImage(img2, "source", WINDOW_WIDTH, WINDOW_HEIGHT);
	

	waitKey(0);
	int a = 0;
	cin >> a;
	return 0;
}
