#include "cv-common.hpp"

struct regionState : NeighborState {
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

void findRegion(Mat& mat, u8 markAs, Filter filt, points& posns)
/* Use region-growing to expand posns. The filter determines which
 * pixels to include in the region. Each pixel is marked (markAs). */
{
	int visited = 0;
	regionState state = regionState(posns, filt, markAs);
	while (state.to_visit > visited) {
		Vec2i cur = posns[visited];
		int ci = cur[0], cj = cur[1];
		applyToNeighbors(mat, &state, ci, cj);
		++visited;
	}
}

struct borderState : NeighborState {
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
	applyToNeighbors(mat, &state, ci, cj);
	return state.isBorder;
}

void findBorder(Mat& mat, points& posns, points& border)
{
	for (size_t k=0; k < posns.size(); ++k) {
		if (isBorderPixel(mat, posns[k])) {
			border.push_back(posns[k]);
		}
	}
}

Point2i findCentroid(Mat& mat, points& posns)
/* http://mathworld.wolfram.com/GeometricCentroid.html */
{
	int xsum = 0, ysum = 0;
	for (size_t k=0; k < posns.size(); ++k) {
		xsum += posns[k][0];
		ysum += posns[k][1];
	}
	return Point2i(xsum / posns.size(), ysum / posns.size());
}

int findRadius(Point2i centroid, points& boundary)
/* Averages the distances from the centroid to the region boundary. */
{
	double sumRadius = 0;
	int cx = centroid.x, cy = centroid.y;
	for (size_t k=0; k < boundary.size(); ++k) {
		sumRadius += dist(cx, boundary[k][0], cy, boundary[k][1]);
	}
	return int(round(sumRadius / posns.size()));
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
	Scalar outline = Scalar(0, 0, 0, 0);
	FOR_EACH_PIXEL(regions, i, j, {
		if (shadeFilter(regions.at<u8>(i, j))) {
			posns.push_back(Vec2i(i, j));
			int color = randRange(160, 192);
			findRegion(regions, color, shadeFilter, posns);
			posns.clear();
		}
	});
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
