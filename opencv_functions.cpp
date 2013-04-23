#include <highgui.h>
#include <cv.h>
#include <time.h>
#include <string.h>
#include <sqlite3.h>
#include <string>

#define MAX_DATE 20 

using namespace cv;
using namespace std;

static string image_location = "tmp/";
sqlite3 *db;

string get_date(void) {
	time_t now;
	char the_date[MAX_DATE];

	the_date[0] = '\0';
	now = time(NULL);
	if( now != -1)
		strftime(the_date,MAX_DATE,"%y_%m_%d_%H-%M-%S", localtime(&now));

	return string(the_date);
}
int init_db() {
	// // Execute the query for creating the table
	if(sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS intruders (id INTEGER PRIMARY KEY AUTOINCREMENT,file TEXT,detected BOOLEAN DEFAULT FALSE,t TIMESTAMP DEFAULT CURRENT_TIMESTAMP)",0,0,0) != SQLITE_OK){
		printf("failed to create table\n");
		return 1;
	}
	return 0;

}

bool open_db(const char *filename) {
	if(sqlite3_open(filename,&db) == SQLITE_OK)
		return true;
	return false;
}

bool clean_db() {
	if(sqlite3_exec(db,"TRUNCATE intruders",0,0,0) != SQLITE_OK) {
		printf("failed to empty table\n");
		return false;
	}
	return true;
}

int insert_db(const char* fileloc) {
	sqlite3_stmt *statement;

	if(sqlite3_prepare_v2(db,"INSERT INTO intruders (file) VALUES (?)",-1,&statement,0) != SQLITE_OK) {
		printf("Could not prepare statement\n");
		return 1;
	}

	if(sqlite3_bind_text(statement,1,fileloc,strlen(fileloc),SQLITE_STATIC) != SQLITE_OK) {
		printf("Could not bind int\n");
		return 1;
	}

	if (sqlite3_step(statement) != SQLITE_DONE) {
		printf("\nCould not step (execute) stmt.\n");
		return 1;
	}

	return 0;
}



void check_motion(Mat& a, Mat& b) {
    // Blur images to reduce noise
    Mat a_blurred, b_blurred;
    blur(a, a_blurred, Size(4,4));
    blur(b, b_blurred, Size(4,4));

    // Get absolute difference image
    Mat c;
    absdiff(b_blurred, a_blurred, c);

    // Split image to each channels
    vector<Mat> channels;
    split(c, channels);

    // Apply threshold to each channel and combine the results
    Mat d = Mat::zeros(c.size(), CV_8UC1);
    for (int i = 0; i < channels.size(); i++) {
        Mat thresh;
        threshold(channels[i], thresh, 45, 255, CV_THRESH_BINARY);
        d |= thresh;
    }

    // Perform morphological close operation to filling in the gaps
    Mat kernel, e;
    getStructuringElement(MORPH_RECT, Size(10,10));
    morphologyEx(d, e, MORPH_CLOSE, kernel, Point(-1,-1), 5);

    // Find all contours
    vector<vector<Point> > contours;
    findContours(e.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    // Select only large enough contours
    vector<vector<Point> > intruders;
    for (int i = 0; i < contours.size(); i++) {
        double area = contourArea(contours[i]);
        if (area > 10000 && area < 90000)
            intruders.push_back(contours[i]);
    }

    // Highlight the foreground object by darken the rest of the image
    // Intruder detected
    if (intruders.size()) {
	printf("intruder detected!\n");
	string fileloc = image_location + "intruder_" + get_date() + ".jpg";
	insert_db(fileloc.c_str());
	imwrite(fileloc.c_str(),b);	
    }
}


void intruderAlarm(Mat a, Mat b)
{
    // Blur images to reduce noise
    Mat a_blurred, b_blurred;
    blur(a, a_blurred, Size(4,4));
    blur(b, b_blurred, Size(4,4));

    // Get absolute difference image
    Mat c;
    absdiff(b_blurred, a_blurred, c);

    // Split image to each channels
    vector<Mat> channels;
    split(c, channels);

    // Apply threshold to each channel and combine the results
    Mat d = Mat::zeros(c.size(), CV_8UC1);
    for (int i = 0; i < channels.size(); i++)
    {
        Mat thresh;
        threshold(channels[i], thresh, 45, 255, CV_THRESH_BINARY);
        d |= thresh;
    }

    // Perform morphological close operation to filling in the gaps
    Mat kernel, e;
    getStructuringElement(MORPH_RECT, Size(10,10));
    morphologyEx(d, e, MORPH_CLOSE, kernel, Point(-1,-1), 5);

    // Find all contours
    vector<vector<Point> > contours;
    findContours(e.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    // Select only large enough contours
    vector<vector<Point> > intruders;
    for (int i = 0; i < contours.size(); i++) 
    {
	
        double area = contourArea(contours[i]);
        if (area > 10000 && area < 90000)
            intruders.push_back(contours[i]);
    }

    // Use the filtered blobs above to create a mask image to 
    // extract the foreground object
    Mat mask = Mat::zeros(b.size(), CV_8UC3);
    drawContours(mask, intruders, -1, CV_RGB(255,255,255), -1);

    // Highlight the foreground object by darken the rest of the image
    if (intruders.size())
    {
	printf("intruders\n");
        b = (b/4 & ~mask) + (b & mask);
        drawContours(b, intruders, -1, CV_RGB(255,255,255), 2);
    }
}
