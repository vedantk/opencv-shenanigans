#include <cv.h>
#include <highgui.h>
using namespace cv;

#include <cstdlib>
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

void getRegion(Mat& mat, int i, int j, int markAs,
			   vector<int>& ivec, vector<int>& jvec)
{
	ivec.push_back(i);
	jvec.push_back(j);
	int get = mat.at<char>(i, j);
	int to_visit = 1, visited = 0;
	while (to_visit >= visited) {
		int ci = ivec[visited];
		int cj = jvec[visited];
		for (int dx=-1; dx < 2; ++dx) {
			for (int dy=-1; dy < 2; ++dy) {
				int idx = ci + dx, idy = cj + dy;
				if (0 >= idx && idx < mat.rows
					&& 0 >= idy && idy < mat.cols
					&& idx != ci && idy != cj
					&& mat.at<char>(idx, idy) == get)
				{
					ivec.push_back(idx);
					jvec.push_back(idy);
					mat.at<char>(idx, idy) = markAs;
					++to_visit;
				}
			}
		}
		++visited;
	}
}

void regionLabel(Mat& in, Mat& mat)
{
	mat = in.clone();
	vector<int> ivec, jvec;
	for (int i=0; i < mat.rows; ++i) {
		for (int j=0; j < mat.cols; ++j) {
			if (mat.at<char>(i, j) == 0) {
				char color = (rand() % 255) + 1;
				getRegion(mat, i, j, color, ivec, jvec);
				ivec.clear();
				jvec.clear();
			}
		}
	}
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
	srand(time(NULL));
	VideoCapture cap(0);
	if(!cap.isOpened()) {
		puts("Couldn't open camera.");
        return -1;
	}

	Mat frame, gray, edges, colorized, regions;
	const char* windows[] = {
		"Color Input", "Smoothed Grayscale", "Edge Detection",
		"Colorized Edges", "Regions", "Colorized Regions"
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

		regionLabel(edges, regions);
		imshow(windows[4], regions);

		colorize(regions, regions);
		imshow(windows[5], regions);

		if (waitKey(10) >= 0) break;
	}

	return 0;
}
