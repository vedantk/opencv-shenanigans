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
			posns.push_back(Point(idx, idy));
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
		Point cur = posns[visited];
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
		} else return true;
	}
};

inline bool isBorderPixel(Mat& mat, Point pt, borderState state)
{
	state.isBorder = false;
	applyToNeighbors(mat, &state, pt);
	return state.isBorder;
}

void findBorder(Mat& mat, points& posns, points& border)
{
	borderState state = borderState(mat.at<u8>(posns.front()));
	for (size_t k=0; k < posns.size(); ++k) {
		if (isBorderPixel(mat, posns[k], state)) {
			border.push_back(posns[k]);
		}
	}
}

Point findCentroid(points& posns)
/* http://mathworld.wolfram.com/GeometricCentroid.html */
{
	int xs = 0, ys = 0;
	for (size_t k=0; k < posns.size(); ++k) {
		xs += posns[k].x;
		ys += posns[k].y;
	}
	return Point(fdiv(xs, posns.size()), fdiv(ys, posns.size()));
}

int findRadius(Point centroid, points& border)
/* Averages the distances from the centroid to the region border. */
{
	double sumRadius = 0;
	for (size_t k=0; k < border.size(); ++k) {
		sumRadius += dist(centroid, border[k]);
	}
	return fdiv(sumRadius, border.size());
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
			areas.push_back(points());
			points& area = areas.back();
			area.push_back(Point(i, j));
			int color = randRange(160, 192);
			findRegion(regions, color, shadeFilter, area);
		}
	});
}

void processRegions(Mat& regions, vector<points>& areas)
{
	const Scalar scale = Scalar(0, 0, 0, 0);
	for (size_t k=0; k < areas.size(); ++k) {
		size_t sz = areas[k].size();
		if (sz < 10 || sz > 50) continue;

		points border;
		Point pt = findCentroid(areas[k]);
		findBorder(regions, areas[k], border);
		for (size_t i=0; i < border.size(); ++i) {
			regions.at<u8>(border[i]) = 0;
		}
	}
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
		Canny(gray, edges, 0, 50, 3, false);
		colorize(gray, colorized);

		vector<points> areas;
		regionLabel(edges, regions, areas);
		processRegions(regions, areas);

		imshow(windows[0], frame);
		imshow(windows[1], gray);
		imshow(windows[2], regions);
		imshow(windows[3], colorized);
		if (waitKey(10) >= 0) break;
		// waitKey(0);
	}
	return 0;
}
