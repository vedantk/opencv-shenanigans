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
			posns.push_back(Point2i(idx, idy));
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
		Point2i cur = posns[visited];
		applyToNeighbors(mat, &state, cur);
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

inline bool isBorderPixel(Mat& mat, Point2i pt)
{
	borderState state = borderState(mat.at<u8>(pt));
	applyToNeighbors(mat, &state, pt);
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
		xsum += posns[k].x;
		ysum += posns[k].y;
	}
	return Point2i(xsum / posns.size(), ysum / posns.size());
}

int findRadius(Point2i centroid, points& border)
/* Averages the distances from the centroid to the region border. */
{
	double sumRadius = 0;
	int cx = centroid.x, cy = centroid.y;
	for (size_t k=0; k < border.size(); ++k) {
		sumRadius += dist(cx, border[k].x, cy, border[k].y);
	}
	return int(round(sumRadius / border.size()));
}

bool shadeFilter(u8 shade)
{
	return shade < 32;
}

void regionLabel(Mat& edges, Mat& regions, vector<points>& areas)
/* Randomly color edge-separated regions. */
{
	regions = edges.clone();
	FOR_EACH_PIXEL(regions, i, j, {
		if (shadeFilter(regions.at<u8>(i, j))) {
			points area;
			area.push_back(Point2i(i, j));
			int color = randRange(160, 192);
			findRegion(regions, color, shadeFilter, area);
			areas.push_back(area);
		}
	});
}

void processRegions(Mat& regions, vector<points>& areas)
{
	points border;
	Scalar scale = Scalar(0, 0, 0, 0);
	for (size_t k=0; k < areas.size(); ++k) {
		size_t sz = areas[k].size();
		if (sz < 20 || sz > 40) continue;
		Point2i pt = findCentroid(regions, areas[k]);
		findBorder(regions, areas[k], border);
		int radius = findRadius(pt, border);
		for (size_t i=0; i < border.size(); ++i) {
			regions.at<u8>(border[i]) = 255;
		}
		circle(regions, pt, radius / 5, scale);
		circle(regions, pt, radius, scale);
		border.clear();
	}
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
	vector<points> areas;
	while (true) {
		cap >> frame;
		cvtColor(frame, gray, CV_BGR2GRAY);
		GaussianBlur(gray, gray, Size(3, 3), 5, 5);
		Canny(gray, edges, 0, 50, 3, true);
		colorize(gray, colorized);

		regionLabel(edges, regions, areas);
		processRegions(regions, areas);
		areas.clear();

		imshow(windows[0], frame);
		imshow(windows[1], gray);
		imshow(windows[2], regions);
		imshow(windows[3], colorized);
		waitKey(0);
		if (waitKey(10) >= 0) break;
	}
	return 0;
}
