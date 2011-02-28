#include <cv.h>
#include <highgui.h>
using namespace cv;

#include <iostream>
using namespace std;

void matStats(Mat& mat)
{
	cout
		<< "mat.depth() = " << mat.depth() << endl
		<< "mat.channels() = " << mat.channels() << endl
		<< "mat.dims = " << mat.dims << endl
		<< "mat.elemSize() = " << mat.elemSize()
		<< endl;
	cin.get();
}

void colorize(Mat& mat, Mat& out)
{
	vector<Mat> planes;
	for (int k=0; k < 3; ++k) {
		planes.push_back(mat.clone());
	}
	for (int i=0; i < mat.rows; ++i) {
		for (int j=0; j < mat.cols; ++j) {
			planes[0].at<char>(i, j) &= 0xFF;
			planes[1].at<char>(i, j) ^= 0xFF;
			planes[2].at<char>(i, j) |= 0xFF;
		}
	}
	merge(planes, out);
}

int main(int argc, char** argv)
{
	VideoCapture cap(0);
	if(!cap.isOpened()) {
		puts("Couldn't open camera.");
        return -1;
	}

	Mat frame, gray, edges, colorized;
	const char* windows[] = {
		"Color Input", "Smoothed Grayscale", "Edge Detection",
		"Colorized Edges"
	};
	for (size_t i=0; i < sizeof(windows)/sizeof(char*); ++i) {
		namedWindow(windows[i], 1);
	}

	while (true) {
		cap >> frame;
		imshow(windows[0], frame);

		cvtColor(frame, gray, CV_BGR2GRAY);
		GaussianBlur(gray, gray, Size(7, 7), 3, 3);
		imshow(windows[1], gray);

		Canny(gray, edges, 0, 30, 3, true);
		imshow(windows[2], edges);

		colorize(edges, colorized);
		imshow(windows[3], colorized);

		if (waitKey(10) >= 0) break;
	}

	return 0;
}
