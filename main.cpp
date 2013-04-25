#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "opencv_functions.cpp"


using namespace cv;

/* TODO:
 * add config file: http://developer.gnome.org/glib/unstable/glib-Key-value-file-parser.html
 *
 *
 */

int main(int argc, char const* argv[])
{

	// Vars
	const char *db_name = "database.db";
	open_db(db_name);
	init_db();

	/* OpenCV */
	Mat prev;
	Mat cur;
	Mat next;

	VideoCapture cap(0); // open the default camera
	if(!cap.isOpened())  // check if we succeeded
		return -1;

	while (1) {
		cap >> cur;

		if(cur.data && prev.data){
			check_motion(prev,cur);
		}

		cur.copyTo(prev);
	}

	close_db();
	return 0;

}

