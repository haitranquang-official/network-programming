#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "file_transfer.h"
#include "../database/connection.h"
#include "../database/user.h"

char DATA_START[128] = "150 Start transfering data on the data channel\n"; 
char DATA_COMPLETED[128] = "226 Successfully sent/receive\n";

void finish_with_error(MYSQL* connection) {
	fprintf(stderr, "%s\n", mysql_error(connection)); 
	close_connection();
}

void wait_client_response(int fd) {
	char ok_from_client[128];
	memset(ok_from_client, 0, sizeof(ok_from_client));
	recv(fd, ok_from_client, sizeof(ok_from_client), 0);
}

void insert_new_resource(int user_id, char* dir_path, char* resource_name) {
	MYSQL* connection = open_connection();

	char query[2048];
	memset(query, 0, sizeof(query));

	sprintf(query, "INSERT INTO resource (user_id, path) values (%d, %s/%s)", user_id, dir_path, resource_name);

	if(mysql_query(connection, query)) {
		finish_with_error(connection);
	}
}

int upload(int cfd, int dfd, int user_id, char* file_name, char* upload_path) {
	// pre-process file_name
	while(	file_name[strlen(file_name) - 1] == '\r' ||
			file_name[strlen(file_name) - 1] == '\n') {
		file_name[strlen(file_name) - 1] = 0;
	}

	// find home dir of user 
	char* home_dir = find_home_dir_by_user(user_id);
	if(home_dir == NULL) {
		fprintf(stderr, "Something wrong with finding home_dir\n");
	}

	// create path to which the file is uploaded
	char full_path[1024];
	memset(full_path, 0, sizeof(full_path));
	sprintf(full_path, "%s%s/%s", home_dir, upload_path, file_name);

	// send notification to client
	send(cfd, DATA_START, strlen(DATA_START), 0);

	FILE* file = fopen(full_path, "wb");
	char data[1024];
	while(1) {
		memset(data, 0, sizeof(data));

		// Receive the file from client (receive at max rate).
		int r = recv(dfd, data, sizeof(data), 0);
		if(r > 0) {
			fwrite(data, sizeof(char), r, file);	
		}

		// if this is the last chunk from the client, then break
		if(r < sizeof(data)) {
			break;
		}	
	}
	fclose(file);

	close(dfd);

	// insert a new record to table `resource`
	insert_new_resource(user_id, upload_path, file_name);

	send(cfd, DATA_COMPLETED, strlen(DATA_COMPLETED), 0);

	free(home_dir);

	return 0;
}

int download(int cfd, int dfd, int user_id, char* file_path) {
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
				sprintf(abosulute_user_path, "%s%s", row[i], file_path);			
			}
			else {
				fprintf(stderr, "No home_dir found for user %d", user_id);
			}
		}
	}

	mysql_free_result(result);

	// find absolute path from database
	// char absolute_system_path[2048];
	// memset(absolute_system_path, 0, sizeof(absolute_system_path));

	// find the real file path in the system
	// sprintf(query, "select path from resource where path like \"%%%s\" and user_id = %d", abosulute_user_path, user_id);
	// if(mysql_query(connection, query)) {
	// 	finish_with_error(connection);
	// 	return 1;
	// }

	// result = mysql_store_result(connection);
	// row = mysql_fetch_row(result);

	// if(row != NULL) {
		send(cfd, DATA_START, strlen(DATA_START), 0);
		wait_client_response(cfd);

		// FILE* file = fopen(row[0], "rb");
		FILE* file = fopen(abosulute_user_path, "rb");	

		fseek(file, 0, SEEK_END);
		int size = ftell(file);
		fseek(file, 0, SEEK_SET);

		char* data = (char *) calloc(size, sizeof(char));
		fread(data, 1, size, file);

		char content_size[256];
		memset(content_size, 0, sizeof(content_size));
		sprintf(content_size, "%s%d\n", "Content Length: ", size);

		send(dfd, content_size, strlen(content_size), 0);
		wait_client_response(dfd);

		// send this file to client
		int sent = 0;
		while(sent < size) {
			sent += send(dfd, data + sent, size - sent, 0);
		}
		close(dfd);

		wait_client_response(dfd);

		free(data);
		data = NULL;

		fclose(file);

		send(cfd, DATA_COMPLETED, strlen(DATA_COMPLETED), 0);
	// }

	// mysql_free_result(result);

	return 0;
}