#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include "opencv_functions.cpp"
#include <glib.h>

using namespace std;
using namespace cv;


int main(int argc, char const *argv[])
{
	// Config file options
	Settings *conf;
	GKeyFile *keyfile;
	GKeyFileFlags flags;
	GError *error = NULL;
	gsize length;

   	string default_path = "/rpisecsys/config";
	
	// Create a new GKeyFile object and a bitwise list of flags. 
	keyfile = g_key_file_new();	
	if(!g_key_file_load_from_file(keyfile,
		(g_get_user_config_dir() + default_path).c_str(),flags,&error)) {
		printf("Failed to load config file\n");
		return -1;
	}

	conf = g_slice_new(Settings);
	conf->database = g_key_file_get_string(keyfile,"options","database",NULL);
	conf->image_directory = g_key_file_get_string(keyfile,"options","image_directory",NULL);
	conf->nma_key = g_key_file_get_string(keyfile,"nma","api_key",NULL);
	conf->email_to = g_key_file_get_string(keyfile,"email","to",NULL);
	conf->email_subject = g_key_file_get_string(keyfile,"email","subject",NULL);

	// Open database
	open_db(conf->database);
	init_db();

	// OpenCV 
	Mat prev,cur,original;
	VideoCapture cap(0); 

	// Setup capturing
	if(!cap.isOpened()){  
		printf("Failed to capture from camera\n");
		return -1;
	}

	// Capture first image 
	// Original will be the image that is presented to the user
	cap >> original;
	cur = original;
    	cvtColor(cur,cur,CV_RGB2GRAY);

	init_facedetection();
	string fileloc;

	while(1) {
		if(cur.data && prev.data){

			// Check if motion exists
			if(motion(cur,prev)) {
				printf("motion detected!\n");
				fileloc = (string)conf->image_directory + "intruder_" + get_date() + ".jpg"; 
				imwrite(fileloc.c_str(),original);	

				if(conf->nma_key != NULL)
					sendmail(conf->email_subject,conf->email_to,fileloc.c_str());
				if(conf->email_to != NULL && conf->email_subject != NULL)
					notifymyandroid(conf->nma_key,get_date().c_str());

				// Try to detect a face
				if(detect_face(cur))
					insert_db(fileloc.c_str(),true);
				else
					insert_db(fileloc.c_str(),false);
			}
			else 
				cvWaitKey(600);
		}

		// Copy image to prev, grab a new image
		cur.copyTo(prev);
		cap >> cur;
    		cvtColor(cur,cur,CV_RGB2GRAY);
	}
	return 0;
}
