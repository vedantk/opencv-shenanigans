#include <cv.h>
#include <highgui.h>
using namespace cv;

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

void getRegion(Mat& mat, bool (*lambda)(u8 shade), u8 markAs,
			   vector<int>& ivec, vector<int>& jvec)
{
	int visited = 0;
	int to_visit = ivec.size();
	while (to_visit > visited) {
		int ci = ivec[visited];
		int cj = jvec[visited];
		++visited;

		for (int dx=-1; dx < 2; ++dx) {
			for (int dy=-1; dy < 2; ++dy) {
				int idx = ci + dx, idy = cj + dy;
				if (idx >= 0 && idx < mat.rows
					&& idy >= 0 && idy < mat.cols
					&& idx != ci && idy != cj
					&& lambda(mat.at<u8>(idx, idy)))
				{
					ivec.push_back(idx);
					jvec.push_back(idy);
					mat.at<u8>(idx, idy) = markAs;
					++to_visit;
				}
			}
		}
	}
}

int randRange(int a, int b)
{
	return (rand() % (b + 1)) + a;
}

bool shadeFilter(u8 shade)
{
	return shade < 32;
}

void regionLabel(Mat& edges, Mat& regions)
{
	regions = edges.clone();
	vector<int> ivec, jvec;
	for (int i=0; i < regions.rows; ++i) {
		for (int j=0; j < regions.cols; ++j) {
			if (shadeFilter(regions.at<u8>(i, j))) {
				ivec.push_back(i);
				jvec.push_back(j);
				u8 color = randRange(32, 192);
				getRegion(regions, shadeFilter, color, ivec, jvec);
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
			planes[0].at<u8>(i, j) = (i * j) % 256;
			planes[1].at<u8>(i, j) ^= 0xFF;
			planes[2].at<u8>(i, j) = 0xFF - mat.at<u8>(i, j);
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

	const char* windows[] = {
		"Color Input", "Smoothed Grayscale",
		"Edge Detection", "Regions", "Colorized Regions"
	};
	for (size_t i=0; i < sizeof(windows)/sizeof(char*); ++i) {
		namedWindow(windows[i], 1);
	}

	Mat frame, gray, edges, regions, colorized;
	while (true) {
		cap >> frame;
		cvtColor(frame, gray, CV_BGR2GRAY);
		GaussianBlur(gray, gray, Size(3, 3), 2, 2);
		Canny(gray, edges, 0, 30, 3, true);
		regionLabel(edges, regions);
		colorize(regions, colorized);

		imshow(windows[0], frame);
		imshow(windows[1], gray);
		imshow(windows[2], edges);
		imshow(windows[3], regions);
		imshow(windows[4], colorized);
		if (waitKey(10) >= 0) break;
	}

	return 0;
}
