#include <stdio.h>
#include <string.h>
#include "../database/connection.h"

char* find_home_dir_by_user(int user_id) {
	MYSQL* connection = open_connection();

	char query[2048];
	memset(query, 0, sizeof(query));

	// concatenate file_path to user's home_dir
	sprintf(query, "SELECT home_dir FROM user WHERE id = %d", user_id);

	// if the query fails, then close the connection and return
	if(mysql_query(connection, query)) {
		fprintf(stderr, "%s\n", mysql_error(connection)); 
		close_connection();
		return NULL;
	}

	// if not fails, then return the home_dir 
	MYSQL_RES* result = mysql_store_result(connection);
	if(result == NULL) {
		fprintf(stderr, "%s\n", mysql_error(connection)); 
		close_connection();
		return NULL;
	}

	MYSQL_ROW row;
	row = mysql_fetch_row(result);

	if(row == NULL) {
		return NULL;
	}

	char* res = (char*) malloc(1024 * sizeof(char)); 
	strcpy(res, row[0]);

	mysql_free_result(result);
	
	return res; 
}