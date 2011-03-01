#include <cv.h>
#include <highgui.h>
using namespace cv;

#include <cmath>
#include <cstdlib>
#include <iostream>
using namespace std;

typedef unsigned char u8;

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

void getRegion(Mat& mat, u8 markAs,
			   bool (*lambda)(u8 shade), vector<Vec2i>& posns)
{
	int visited = 0;
	int to_visit = posns.size();
	while (to_visit > visited) {
		Vec2i cur = posns[visited];
		int ci = cur[0], cj = cur[1];
		++visited;

		for (int dx=-1; dx < 2; ++dx) {
			for (int dy=-1; dy < 2; ++dy) {
				int idx = ci + dx, idy = cj + dy;
				if (idx >= 0 && idx < mat.rows
					&& idy >= 0 && idy < mat.cols
					&& idx != ci && idy != cj
					&& lambda(mat.at<u8>(idx, idy)))
				{
					mat.at<u8>(idx, idy) = markAs;
					posns.push_back(Vec2i(idx, idy));
					++to_visit;
				}
			}
		}
	}
}

int randRange(int a, int b)
{
	return a + (abs(rand()) % (b - a));
}

bool shadeFilter(u8 shade)
{
	return shade < 32;
}

void regionLabel(Mat& edges, Mat& regions)
{
	vector<Vec2i> posns;
	regions = edges.clone();
	for (int i=0; i < regions.rows; ++i) {
		for (int j=0; j < regions.cols; ++j) {
			if (shadeFilter(regions.at<u8>(i, j))) {
				posns.push_back(Vec2i(i, j));
				u8 color = randRange(128, 192);
				getRegion(regions, color, shadeFilter, posns);
				posns.clear();
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
			u8 cur = mat.at<u8>(i, j);
			planes[0].at<u8>(i, j) = 0xFF - cur;
			planes[1].at<u8>(i, j) = (cur * cur) % 0xFF;
		}
	}
	merge(planes, out);
}

int main(int argc, char** argv)
{
	srand(time(NULL));
	VideoCapture cap(0);
	if (!cap.isOpened()) {
		puts("Couldn't open camera.");
		return -1;
	}

	const char* windows[] = {"Feed", "Smoothed", "Regions", "Heat"};
	for (size_t i=0; i < sizeof(windows)/sizeof(char*); ++i) {
		namedWindow(windows[i], 1);
	}

	Mat frame, gray, edges, regions, colorized;
	while (true) {
		cap >> frame;
		cvtColor(frame, gray, CV_BGR2GRAY);
		GaussianBlur(gray, gray, Size(3, 3), 5, 5);
		Canny(gray, edges, 0, 50, 3, true);
		regionLabel(edges, regions);
		colorize(gray, colorized);

		imshow(windows[0], frame);
		imshow(windows[1], gray);
		imshow(windows[2], regions);
		imshow(windows[3], colorized);
		if (waitKey(10) >= 0) break;
	}

	return 0;
}
