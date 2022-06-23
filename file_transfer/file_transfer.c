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

int download(struct file_transfer_param param) {
	char* file_path = param.file_path;
	int user_id = param.user_id;

	char abosulute_user_path[1024];
	memset(abosulute_user_path, 0, sizeof(abosulute_user_path));

	MYSQL* connection = open_connection();

	char query[2048];
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
				sprintf(abosulute_user_path, "%s/%s", row[i], file_path);			
			}
			else {
				fprintf(stderr, "No home_dir found for user %d", user_id);
			}
		}
	}

	printf("%s\n", abosulute_user_path);

	mysql_free_result(result);

	// find absolute path from database
	char absolute_system_path[2048];
	memset(absolute_system_path, 0, sizeof(absolute_system_path));

	// find the real file path in the system
	sprintf(query, "select path from resource where path like \"%%%s\" and user_id = %d", abosulute_user_path, user_id);
	if(mysql_query(connection, query)) {
		finish_with_error(connection);
		return 1;
	}

	result = mysql_store_result(connection);
	row = mysql_fetch_row(result);

	if(row != NULL) {
		FILE* file = fopen(row[0], "rb");	

		fseek(file, 0, SEEK_END);
		int size = ftell(file);
		fseek(file, 0, SEEK_SET);

		char* data = (char *) calloc(size, sizeof(char));
		fread(data, 1, size, file);
		printf("%s\n", data);

		free(data);
		data = NULL;

		fclose(file);
	}

	mysql_free_result(result);

	// create a file in local

	// write file
}