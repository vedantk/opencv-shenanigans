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

void findEdges(Mat& gray, Mat& out)
{
	int hist[0xFF] = {0};
	int size = gray.rows * gray.cols;
	for (int i=0; i < size; ++i) {
		hist[gray.data[i]] += 1;
	}
	int max = 0, min=0;
	for (int i=1; i < 0xFF; ++i) {
		max = (hist[i] > hist[max]) ? i : max;
		min = (hist[i] < hist[min]) ? i : min;
	}
	Canny(gray, out, min, max, 3, true);
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

template<typename State>
void applyToNeighbors(Mat& mat, State* data, int ci, int cj)
/* Call data->apply on all of the neighbors of the point (ci, cj).
 * If the function evaluates to false, the loop is broken. */
{
	for (int dx=-1; dx < 2; ++dx) {
		for (int dy=-1; dy < 2; ++dy) {
			int idx = ci + dx, idy = cj + dy;
			if (idx >= 0 && idx < mat.rows
				&& idy >= 0 && idy < mat.cols
				&& idx != ci && idy != cj)
			{
				if (!data->apply(mat, idx, idy))
					return;
			}
		}
	}
}

struct regionState {
	int to_visit;
	points& posns;
	Filter lambda;
	u8 markAs;

	regionState(points& li, Filter filt, u8 flag)
		: posns(li), lambda(filt), markAs(flag)
	{
		to_visit = li.size();
	}

	bool apply(Mat& mat, int idx, int idy)
	{
		if (lambda(mat.at<u8>(idx, idy))) {
			mat.at<u8>(idx, idy) = markAs;
			posns.push_back(Vec2i(idx, idy));
			++to_visit;
		}
		return true;
	}
};

void getRegion(Mat& mat, u8 markAs, Filter filt, points& posns)
/* Given points in posns, find their surrounding homogenous region.
 * The filter checks if new points should be included in the region. */
{
	int visited = 0;
	regionState state = regionState(posns, filt, markAs);
	while (state.to_visit > visited) {
		Vec2i cur = posns[visited];
		int ci = cur[0], cj = cur[1];
		applyToNeighbors<regionState>(mat, &state, ci, cj);
		++visited;
	}
}

struct borderState {
	u8 shade;
	bool isBorder;

	borderState(u8 flag)
		: shade(flag), isBorder(false)
	{}

	bool apply(Mat& mat, int idx, int idy)
	{
		if (mat.at<u8>(idx, idy) != shade) {
			isBorder = true;
			return false;
		}
		return true;
	}
};

inline bool isBorderPixel(Mat& mat, Vec2i posn)
{
	int ci = posn[0], cj = posn[1];
	borderState state = borderState(mat.at<u8>(ci, cj));
	applyToNeighbors<borderState>(mat, &state, ci, cj);
	return state.isBorder;
}

Vec2i findCenter(Mat& mat, points posns)
{
	points border;
	for (size_t i=0; i < posns.size(); ++i) {
		if (isBorderPixel(mat, posns[i])) {
			border.push_back(posns[i]);
		}
	}

	return Vec2i(0, 0);
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
	points posns;
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
	colorize(regions, regions);
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
		findEdges(gray, edges);
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
