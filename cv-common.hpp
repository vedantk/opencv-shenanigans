#include <opencv.hpp>
#include <highgui/highgui.hpp>

using namespace cv;

#include <cmath>
#include <cstdlib>
#include <iostream>
using namespace std;

typedef unsigned char u8;
typedef vector<Point> points;
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
		<< "@ " << mat << endl
		<< "mat.depth() = " << mat.depth() << endl
		<< "mat.channels() = " << mat.channels() << endl
		<< "mat.dims = " << mat.dims << endl
		<< "mat.elemSize() = " << mat.elemSize()
		<< endl;
	cin.get();
}

class NeighborState {
public:
	virtual bool apply(Mat& mat, int idx, int idy) = 0;
};

void applyToNeighbors(Mat& mat, NeighborState* state, Point pt)
/* Call state->apply on all of the neighbors of the point (ci, cj).
 * If the function evaluates to false, the loop is broken. */
{
	for (int dx=-1; dx < 2; ++dx) {
		for (int dy=-1; dy < 2; ++dy) {
			int idx = pt.x + dx, idy = pt.y + dy;
			if (idx >= 0 && idx < mat.rows
				&& idy >= 0 && idy < mat.cols
				&& idx != pt.x && idy != pt.y)
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

double dist(Point lhs, Point rhs)
{
	return sqrt(square(rhs.x - lhs.x) + square(rhs.y - lhs.y));
}

int randRange(int a, int b)
{
	return a + (abs(rand()) % (b - a));
}

inline int fdiv(int lhs, int rhs)
{
	return int(round(double(lhs) / double(rhs)));
}

void printPoint(Point pt)
{
	cout << "[Point] x = " << pt.x << ", " << "y = " << pt.y;
}
