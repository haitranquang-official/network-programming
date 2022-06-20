#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    MYSQL* connection = mysql_init(NULL);

    if (connection == NULL) {
        printf("%s\n", mysql_error(connection));
        exit(1);
    }

    if (mysql_real_connect(connection,
    "localhost", "BUDA", "BUDA", NULL, 0, NULL, 0) == NULL) {
        printf("Invalid credentials");
        mysql_close(connection);
        exit(1);
    } 

    mysql_query(connection, "DROP DATABASE IF EXISTS network_programming");

    mysql_query(connection, "CREATE DATABASE network_programming");

    mysql_query(connection, "USE network_programming");

    mysql_query(connection,
    "DROP TABLE IF EXISTS user");

    mysql_query(connection, 
    "CREATE TABLE user(id INT PRIMARY KEY AUTO_INCREMENT,"
    " username VARCHAR(255) UNIQUE,"
    " password VARCHAR(255),"
    " home_dir VARCHAR(500))");

    mysql_query(connection,
    "DROP TABLE IF EXISTS resource");

    mysql_query(connection,
    "CREATE TABLE resource(id INT PRIMARY KEY AUTO_INCREMENT,"
    " user_id INT NOT NULL, "
    " path VARCHAR(255), "
    " FOREIGN KEY (user_id) REFERENCES user(id))");
}