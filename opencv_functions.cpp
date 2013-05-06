#include "opencv_functions.h"

// face detection
CascadeClassifier face_cascade;
string face_cascade_name = "lbpcascade_frontalface_default.xml";

time_t seconds;

sqlite3 *db;

string get_date() {
	time_t now;
	char the_date[MAX_DATE];

	the_date[0] = '\0';
	now = time(NULL);
	if( now != -1)
		strftime(the_date,MAX_DATE,"%Y_%m_%d_%H-%M-%S", localtime(&now));

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

int insert_db(const char* fileloc,bool face) {
	sqlite3_stmt *statement;

	const char* sql = "INSERT INTO intruders (file) VALUES (?)";
	if(face)
		sql = "INSERT INTO intruders (file,detected) VALUES (?,'TRUE')";
	if(sqlite3_prepare_v2(db,sql,-1,&statement,0) != SQLITE_OK) {
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

bool motion(Mat& a, Mat& b) {

    Mat c;
    blur(a, a, Size(4,4));
    blur(b, b, Size(4,4));
    absdiff(a, b, c);
    Mat d(Size(320,240),CV_8UC1);
    resize(c,d,d.size());

    // Treshold
    threshold(d,d,70,255,CV_THRESH_BINARY);

    // Perform morphological close operation to filling in the gaps
    Mat kernel;
    getStructuringElement(MORPH_RECT, Size(10,10));
    morphologyEx(d, d, MORPH_CLOSE, kernel, Point(-1,-1), 5);

    // Find all contours
    vector<vector<Point> > contours;
    findContours(d.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);


    bool intruder = false;
    for (int i = 0; i < contours.size(); i++) {
        double area = contourArea(contours[i]);
        if (area > 10000 && area < 90000){
		intruder = true;
		break;
	}
    }
    return intruder;
}


bool init_facedetection() {
	if( !face_cascade.load("/usr/share/opencv/lbpcascades/lbpcascade_frontalface.xml")) { 
		printf("Error loading cascade file\n"); 
		return false; 
	}
	return true;
}

bool detect_face(Mat &frame) {
	vector<Rect> faces;

 	face_cascade.detectMultiScale(frame,faces,1.5, 1, CV_HAAR_SCALE_IMAGE |CV_HAAR_DO_CANNY_PRUNING, Size(30,30));
	if(faces.size() > 0)
		return true;
	else 
		return false;
}

bool notifymyandroid(char* api_key, const char* time) {
	return nma_push_msg(api_key, 1,"Raspsurveillance", "Motion detected", "Motion detected ") == 200;
}
