#include <stdio.h>
#include <string.h>
#include "login.h"
#include "../database/connection.h"

int login(char username[256], char password[256]) {
    // Write code to authentication user here
    MYSQL *connection = open_connection();
    char query[2048];
    memset(query, 0, sizeof(query));
    
    sprintf(query, "SELECT id FROM user WHERE username = ('%s') and password = ('%s')", username, password);

    // if the query fails, then close the connection and return
    if (mysql_query(connection, query)) {
		fprintf(stderr, "%s\n", mysql_error(connection)); 
		close_connection();
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(connection);
    if (result == NULL) {
		fprintf(stderr, "%s\n", mysql_error(connection)); 
		close_connection();
        return -1;
    }

    MYSQL_ROW row;
    row = mysql_fetch_row(result);

    if (row == NULL) {
		fprintf(stderr, "%s\n", mysql_error(connection)); 
		close_connection();
        return -1;
    }

    mysql_free_result(result);

    return atoi(row[0]);
}