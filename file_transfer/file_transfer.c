#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include "file_transfer.h"
#include "../database/connection.h"

char DATA_START[128] = "150 Start transfering data on the data channel"; 


void finish_with_error(MYSQL* connection) {
	fprintf(stderr, "%s\n", mysql_error(connection)); 
	close_connection();
}

int upload(char* file_path) {

}

// Client: DOWNLOAD <FILENAME>
int download(char* file_path, int user_id) {
	char abosulute_path[1024];
	memset(abosulute_path, 0, sizeof(abosulute_path));

	MYSQL* connection = open_connection();

	char query[1024];
	memset(query, 0, sizeof(query));

	// concatenate file_path to user's home_dir
	sprintf(query, "SELECT home_dir FROM user WHERE id = %d", user_id);

	// if the query fails, then close the connection and return
	if(mysql_query(connection, query)) {
		finish_with_error(connection);
		return 1;
	}

	// if not fails, then concat file_path to user's home_dir
	MYSQL_RES* result = mysql_store_result(connection);
	if(result == NULL) {
		finish_with_error(connection);
		return 1;
	}

	int num_fields = mysql_num_fields(result);

	MYSQL_ROW row;
	while((row = mysql_fetch_row(result))) {
		for(int i = 0; i < num_fields; i++) {
			if(row[i]) {
				sprintf(abosulute_path, "%s/%s", row[i], file_path);			
			}
			else {
				fprintf(stderr, "No home_dir found for user %d", user_id);
			}
		}
		printf("\n");
	}



	mysql_free_result(result);

	// find absolute path from database
	// sprintf(query, "select * from resources where path like \"%s\" and user_id = %d", file_path, user_id);
	// mysql_query(connection, query);

	// create a file in local

	// write file
}