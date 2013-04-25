#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "opencv_functions.cpp"
#include <sys/types.h>
#include <sys/inotify.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

int main(int argc, char const* argv[])
{
	// Vars
	const char *db_name = "database.db";
	const char *watchdir = "/home/jelle/Projects/RasperryPiSecuritySys/RaspSurveillance";


	// Sqlite
	sqlite3 *db;
	sqlite3_stmt *stmt;

	int retval;

	// Inotify
    	int length, i = 0;
	int fd;
	int wd;
	char buffer[BUF_LEN];

	fd = inotify_init();

	if ( fd < 0 ) 
		perror( "inotify_init" );

    	wd = inotify_add_watch( fd, watchdir, IN_CREATE | IN_MODIFY);

	retval = sqlite3_open(db_name,&db);
	if(retval) {
		printf("Database connection failed\n");
		return -1;
	}

	
	int cols = sqlite3_column_count(stmt);

	// OpenCV
	const char* cascade_name = "/usr/share/opencv/haarcascades/haarcascade_frontalface_default.xml";
	init_detection(cascade_name);
	CvSize min = cvSize(30,30);


	while(1) {
        	length = read( fd, buffer, BUF_LEN );  
		struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
		if(event->len) {
			if(event->mask & IN_MODIFY && strcmp(event->name,db_name)) {
				retval = sqlite3_prepare_v2(db,"SELECT * FROM intruders WHERE PROCESSED = 'FALSE'",-1,&stmt,0);

				while(1 && !retval) {
					retval = sqlite3_step(stmt);

					if(retval == SQLITE_ROW) {
						// ID,file,detected,timestamp,processed
						
						const int id = (const int)sqlite3_column_int64(stmt,0);
						const char *file = (const char*)sqlite3_column_text(stmt,1);
						printf("%d - %s\n",id,file);

						sqlite3_stmt *statement;

						const char *query = "UPDATE intruders SET PROCESSED = 'TRUE' WHERE ID = ?";
						if(detect_and_save(file,min))
							query = "UPDATE intruders SET PROCESSED = 'TRUE', DETECTED = 'TRUE' WHERE ID = ?";

						if(sqlite3_prepare_v2(db,query,-1,&statement,0) == SQLITE_OK) {
							sqlite3_bind_int(statement,1,id);

							// commit
							sqlite3_step(statement);
							sqlite3_finalize(statement);
						}
					}

					else if(retval == SQLITE_DONE)
						break;
					else {
						if(retval != SQLITE_BUSY)
							sqlite3_finalize(stmt);
						printf("Error occured: %d \n",retval);
					}
				}

				sqlite3_finalize(stmt);
			}
		}		
	}
    	( void ) inotify_rm_watch( fd, wd );
    	( void ) close( fd );
}
