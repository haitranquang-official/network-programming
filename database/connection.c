#include <stdio.h>
#include "connection.h"

MYSQL* connection = NULL;

// database credentials
char* host = "localhost";
char* username = "debian-sys-maint";
char* password = "9sbcURb7H6tvLG16";
char* database = "test";

MYSQL* open_connection() {
	// initialize connection
	if(connection == NULL) {
		connection = mysql_init(NULL);
		if (connection == NULL) {
			printf("%s\n", mysql_error(connection));
			exit(1);
    	}
	}

	// try connecting to the database
	if (mysql_real_connect(connection, host, username, password, database, 0, NULL, 0) == NULL) {
        printf("Invalid credentials");
        exit(1);
    }

	return connection;
}

void close_connection() {
	mysql_close(connection);
	connection = NULL;
}