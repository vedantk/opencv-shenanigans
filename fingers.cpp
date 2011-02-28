#include <cv.h>
#include <highgui.h>

using namespace cv;

const char* names[] = {"one", "two", "three", "four", "five"};

int main(int argc, char** argv)
{

	for (int i=0; i < 5; ++i) {
		string fname = "train-fingers/" + string(names[i]) + ".png";
		Mat img = imread(fname, 0);
		GaussianBlur(img, img, Size(7, 7), 3, 3);
		Canny(img, img, 0, 30, 3);
		namedWindow("test", 1);
		imshow("test", img);
		waitKey(0);
	}


	return 0;
}
