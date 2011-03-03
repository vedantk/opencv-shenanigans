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

struct centroidState {
	points& border;
	points orderedSet;
	u8 flag;

	centroidState(points& boundary)
		: border(boundary)
	{
		flag = border[0]
	}

	bool apply(Mat& mat, int idx, int idy)
	{


Vec2i findCentroid(Mat& mat, points posns)
/* http://en.wikipedia.org/wiki/Centroid#Centroid_of_polygon */
{
	points border;
	Mat boundaryMap = Mat(mat.rows, mat.cols, mat.type);
	centroidState state = centroidState(border);
	for (size_t i=0; i < posns.size(); ++i) {
		if (isBorderPixel(mat, posns[i])) {
			border.push_back(posns[i]);
			boundaryMap.at<u8>(posns[i][0], posns[i][1]) = 1;
		}
	}
	/* order the boundary!
	 * -- pick a point on boundary, find first neighbor, set orig
	 * point to NULL/seen/visited, recurse on neighbor, stop when
	 * no neighbors are on boundary */

	int sumArea = 0, sumX = 0, sumY = 0;
	for (size_t i=0; i < border.size() - 1; ++i) {
		int xi = border[i][0];
		int xii = border[i + 1][0];
		int yi = border[i][1];
		int yii = border[i + 1][1];
		int cnpart = (xi * yii) - (xii * yi);
		sumX += (xi + xii) * cnpart;
		sumY += (yi + yii) * cnpart;
		sumArea += cnpart;
	}
	double reduction = 1 / (3.0 * (sumArea + pow(10, -10)));
	int cx = int(round(reduction * sumX));
	int cy = int(round(reduction * sumY));
	return Vec2i(cx, cy);
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
/* Randomly color edge-separated regions. */
{
	points posns;
	regions = edges.clone();
	for (int i=0; i < regions.rows; ++i) {
		for (int j=0; j < regions.cols; ++j) {
			if (shadeFilter(regions.at<u8>(i, j))) {
				posns.push_back(Vec2i(i, j));
				u8 color = randRange(128, 192);
				getRegion(regions, color, shadeFilter, posns);
				//Vec2i pt = findCentroid(regions, posns);
				//cout << "Region: " << pt[0] << " " << pt[1] << endl;
				//cin.get();
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
