#include <cv.h>
#include <highgui.h>
using namespace cv;

#include <iostream>
using namespace std;

int main(int argc, char** argv)
{
	VideoCapture cap(0);
	if(!cap.isOpened()) {
		puts("Couldn't open camera.");
        return -1;
	}

	Mat frame;
	string fname;
	namedWindow("pic", 1);
	while (true) {
		cap >> frame;
		imshow("pic", frame);
		if (waitKey(50) >= 0) {
			cout << "Specify a save location; ";
			cin >> fname;
			cout << endl;
			imwrite(fname, frame);
		}
	}

	return 0;
}
