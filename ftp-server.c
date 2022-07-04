#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <dirent.h>
#include "./database/connection.h"

pthread_t *tid = NULL;
int count = 0;

void append(char *data, char **presp)
{
    int oldlen = (*presp == NULL ? 0 : strlen(*presp));
    *presp = (char *)realloc(*presp, oldlen + strlen(data) + 1);
    memset(*presp + oldlen, 0, strlen(data) + 1);
    sprintf(*presp + strlen(*presp), "%s", data);
}

// Check client in data base
int checkDataBase(char userName[2000], char password[2000])
{
    printf("%s", userName);
    printf("%s", password);
    // Write code to authentication user here
    MYSQL *connection = open_connection();
    char query[2048];
    memset(query, 0, sizeof(query));
    
    // concatenate file_path to user's home_dir
    sprintf(query, "SELECT id FROM user WHERE username = ('%s') and password = ('%s')", userName, password);
    printf("%s", query);
    // if the query fails, then close the connection and return
    if (mysql_query(connection, query))
    {
        // finish_with_error(connection);
        close_connection();
        return -1;
    }

    // if not fails, then return the home_dir
    MYSQL_RES *result = mysql_store_result(connection);
    if (result == NULL)
    {
        // finish_with_error(connection);
        close_connection();
        return -1;
    }

    int num_fields = mysql_num_fields(result);

    MYSQL_ROW row;
    row = mysql_fetch_row(result);

    if (row == NULL)
    {
        close_connection();
        return -1;
    }

    mysql_free_result(result);

    return atoi(row[0]);
}

// Thread to handle with client
void *thread_proc(void *arg)
{
    int cfd = *((int *)arg);

    close(cfd);
    free(arg);
}

int main(int argc, char **argv)
{
    char userName[1024];
    char password[1024];
    memset(userName, 0, sizeof(userName));
    memset(password, 0, sizeof(password));
    // int res = checkDataBase("haitq", "haitq");
    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in saddr;
    struct sockaddr caddr;    // Bien ra chua dia chi client noi den
    int clen = sizeof(caddr); // Bien vao + ra chua so byte duoc ghi vao caddr
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8080);
    saddr.sin_addr.s_addr = 0; // ANY ADDRESS

    bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr));

    listen(sfd, 10);

    while (1)
    {
        int tmp = accept(sfd, (struct sockaddr *)&caddr, &clen);
        if (tmp >= 0)
        {

            char buffer[1024 * 16];
            memset(buffer, 0, sizeof(buffer));

            int r = recv(tmp, buffer, sizeof(buffer), 0);

            if (r > 0)
            {
                printf("server received: {%s}\n", buffer);
                sscanf(buffer, "userName: %s password: %s", userName, password);

                int result = 0;
                if ((result = checkDataBase(userName, password)))
                { // This is to check the database
                    // Handle each client in a separated thread here
                    int *arg = (int *)calloc(1, sizeof(int));
                    *arg = tmp;

                    tid = (pthread_t *)realloc(tid, (count + 1) * sizeof(pthread_t));

                    // pthread_create(&tid[count], NULL, thread_proc, arg);
                    count++; 
                }
            }
        }
    }
    return 0;
}