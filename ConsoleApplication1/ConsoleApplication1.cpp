// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include "stdafx.h"
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>

using namespace cv;
using namespace std;

void sharp(const Mat &image, Mat& result)
{
	int i, j;

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

		for (j = ch; j < nc - ch; j++)
		{
			cur_line[j] = saturate_cast<uchar>(5 * mid_line[j] - mid_line[j - ch] - mid_line[j + ch] - up_line[j] - down_line[j]);
		}
	}

	result.row(0).setTo(Scalar(0));
	result.row(result.rows - 1).setTo(Scalar(0));
	result.col(0).setTo(Scalar(0));
	result.col(result.cols - 1).setTo(Scalar(0));
}


int main()
{
	Mat img;
	img = imread("C:\\Users\\lukir\\Pictures\\32.png");
	img = imread("C:\\Users\\lukir\\Pictures\\screenshot.png");
	imshow("test", img);

	Mat img2;
	img2.create(img.size(), img.type());

	sharp(img, img2);
	
	imshow("t", img2);	

	waitKey(0);

    return 0;
}
