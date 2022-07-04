#include <stdio.h>
#include "connection.h"

MYSQL* connection = NULL;

// database credentials
char* host = "localhost";
char* username = "BUDA";
char* password = "BUDA";
char* database = "network_programming";

MYSQL* open_connection() {
	// initialize connection
	if(connection == NULL) {
		connection = mysql_init(NULL);
		if (connection == NULL) {
			printf("%s\n", mysql_error(connection));
			exit(1);
    	}
	
		// try connecting to the database
		if (mysql_real_connect(connection, host, username, password, database, 0, NULL, 0) == NULL) {
			fprintf(stderr, "%s\n", mysql_error(connection));
			exit(1);
		}
	}

	return connection;
}

void close_connection() {
	mysql_close(connection);
	connection = NULL;
}