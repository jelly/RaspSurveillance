#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <ctime>
#include "opencv_functions.cpp"
#include <glib.h>

using namespace std;
using namespace cv;


int main(int argc, char const *argv[])
{
	Settings *conf;
	GKeyFile *keyfile;
	GKeyFileFlags flags;
	GError *error = NULL;
	gsize length;
	  
	/* Create a new GKeyFile object and a bitwise list of flags. */
	keyfile = g_key_file_new();	
	if(!g_key_file_load_from_file(keyfile,"rpisecsys.conf",flags,&error)) {
		printf("Failed to load config file\n");
		//g_error(error->message);
		return -1;
	}

	conf = g_slice_new(Settings);
	conf->database = g_key_file_get_string(keyfile,"options","database",NULL);
	conf->image_location = g_key_file_get_string(keyfile,"options","image_location",NULL);

	clock_t t;
	Mat prev,cur;
//	const char *db_name = "database.db";
	open_db(conf->database);
	init_db();

	VideoCapture cap(0); 
	if(!cap.isOpened())  
		return -1;
	Mat original;
	cap >> original;
	cur = original;
    	cvtColor(cur,cur,CV_RGB2GRAY);
	init_facedetection();
	string fileloc;

	while(1) {
		if(cur.data && prev.data){
			t = clock();
			// Check if motion exists
			if(motion(cur,prev)) {

				printf("intruder detected!\n");

				// Try to detect a face
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
