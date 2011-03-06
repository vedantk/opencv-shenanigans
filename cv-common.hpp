#include <cv.h>
#include <highgui.h>
using namespace cv;

#include <cmath>
#include <cstdlib>
#include <iostream>
using namespace std;

typedef unsigned char u8;
typedef vector<Vec2i> points;
typedef bool (*Filter)(u8 shade);

#define FOR_EACH_PIXEL(mat, xvar, yvar, block) \
	for (int xvar=0; xvar < mat.rows; ++xvar) { \
		for (int yvar=0; yvar < mat.cols; ++yvar) { \
			block; \
		} \
	} \

void matInfo(Mat& mat)
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
	FOR_EACH_PIXEL(mat, i, j, {
		u8 cur = mat.at<u8>(i, j);
		planes[0].at<u8>(i, j) = 0xFF - cur;
		planes[1].at<u8>(i, j) = (cur * cur) % 0xFF;
	});
	merge(planes, out);
}

class NeighborState {
public:
	virtual bool apply(Mat& mat, int idx, int idy) = 0;
};

void applyToNeighbors(Mat& mat, NeighborState* state, int ci, int cj)
/* Call state->apply on all of the neighbors of the point (ci, cj).
 * If the function evaluates to false, the loop is broken. */
{
	for (int dx=-1; dx < 2; ++dx) {
		for (int dy=-1; dy < 2; ++dy) {
			int idx = ci + dx, idy = cj + dy;
			if (idx >= 0 && idx < mat.rows
				&& idy >= 0 && idy < mat.cols
				&& idx != ci && idy != cj)
			{
				if (!state->apply(mat, idx, idy))
					return;
			}
		}
	}
}

inline int square(int n)
{
	return n * n;
}

double dist(int x1, int y1, int x2, int y2)
{
	return sqrt(square(x2 - x1) + square(y2 - y1));
}

int randRange(int a, int b)
{
	return a + (abs(rand()) % (b - a));
}
