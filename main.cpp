#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <ctime>
#include "opencv_functions.cpp"

using namespace std;
using namespace cv;


int main(int argc, char const *argv[])
{
	
	clock_t t;
	Mat prev,cur;
	const char *db_name = "database.db";
	open_db(db_name);
	init_db();

	VideoCapture cap(0); // open the default camera
	if(!cap.isOpened())  // check if we succeeded
		return -1;
	Mat original;
	cap >> original;
	cur = original;
    	cvtColor(cur,cur,CV_RGB2GRAY);
	init_facedetection();

	while(1) {
		if(cur.data && prev.data){
			t = clock();
			// If motion 
			if(motion(cur,prev)) {
				printf("intruder detected!\n");
				string fileloc = image_location + "intruder_" + get_date() + ".jpg";
				if(detect_face(cur))
					insert_db(fileloc.c_str(),true);
				else
					insert_db(fileloc.c_str(),false);
				imwrite(fileloc.c_str(),original);	
			}
			else 
				cvWaitKey(600);
			t = clock() -t;
			clock_t Start = clock();
			printf ("%f\n",((float)t)/CLOCKS_PER_SEC);
		}
		cur.copyTo(prev);
		cap >> cur;
    		cvtColor(cur,cur,CV_RGB2GRAY);
	}

	return 0;
}

