#ifndef OPENCVCUSTOM_H_
#define OPENCVCUSTOM_H_

#include <highgui.h>
#include <cv.h>
#include <time.h>
#include <string.h>
#include <sqlite3.h>
#include <string>
#include <glib.h>
#include "hooks/nma.h"

#define MAX_DATE 20 
#define APP_NAME "RPISurveillance"

using namespace std;
using namespace cv;

typedef struct
{
  gchar *database, *image_directory, *nma_key, *email_to, *email_subject;
} Settings;

string get_date();

void sendmail(const char *subject,const char *to,const char* filename);

void close_db();

int init_db();

bool open_db(const char *filename);

bool clean_db();

int insert_db(const char* fileloc,bool face);

bool motion(Mat& a,Mat& b);

bool init_facedetection();

bool detect_face(Mat &frame);

bool notifymyandroid(char* api_key,const char* time);
#endif
