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

// face detection
static CvMemStorage* storage = 0;
static CvMemStorage* storage_haar = 0;
static CvHaarClassifierCascade* cascade = 0;
time_t seconds;

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
	if(sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS intruders (id INTEGER PRIMARY KEY AUTOINCREMENT,file TEXT,detected BOOLEAN DEFAULT FALSE,PROCESSED BOOLEAN DEFAULT FALSE,t TIMESTAMP DEFAULT CURRENT_TIMESTAMP)",0,0,0) != SQLITE_OK){
		printf("failed to create table\n");
		return 1;
	}
	return 0;

}

void close_db() {
	sqlite3_close(db);
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

void init_detection(const char* cascade_name) {
	cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );
	if(!cascade)
		printf("haar filter not found!");

	storage = cvCreateMemStorage(0);
	storage_haar = cvCreateMemStorage(0);
}

bool detect_and_save(const char* file, CvSize facesize) {
    double scale = 1.2;
    bool detected = false;

    IplImage* img = cvLoadImage(file);
    if(!img) 
	    return false;

    // Create a new image based on the input image
    IplImage* temp = cvCreateImage( cvSize(img->width/scale,img->height/scale), 8, 3 );

    // Create two points to represent the face locations
    CvPoint pt1, pt2;
    int i;

    // Clear the memory storage which was used before
    cvClearMemStorage( storage );
    // Find whether the cascade is loaded, to find the faces. If yes, then:
    if(cascade) {
		// There can be more than one face in an image. So create a growable sequence of faces.
		// Detect the objects and store them in the sequence
		CvSeq* faces = cvHaarDetectObjects( img, cascade, storage,
						    1.2, 4,
						    CV_HAAR_DO_CANNY_PRUNING
						    ,
						    facesize);

		// Loop the number of faces found.
		for( i = 0; i < (faces ? faces->total : 0); i++ ) {
		   // Create a new rectangle for drawing the face
		    CvRect* r = (CvRect*)cvGetSeqElem( faces, i );

		    // Find the dimensions of the face,and scale it if necessary
		    pt1.x = r->x*scale;
		    pt2.x = (r->x+r->width)*scale;
		    pt1.y = r->y*scale;
		    pt2.y = (r->y+r->height)*scale;

		    // Draw the rectangle in the input image
		    cvRectangle( img, pt1, pt2, CV_RGB(255,0,0), 3, 8, 0 );
		    string fileloc = "faces/intruder_" + get_date() + ".jpg";
		    if(faces->total > 0)
		    	cvSaveImage(fileloc.c_str(), img );
		    	detected = true;
		}
    }


    // Release the temp image created.
    cvReleaseImage( &temp );
    return detected;
}

