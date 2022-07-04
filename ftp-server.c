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


unsigned short PASVPORT = 10000;

struct THREADPARAM
{
    int cfd;
    int port;
};

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
    char* PORTOK = "250 PORT Okay\n";
    char* CDUPOK = "250 CDUP Okay\n";
    char* CWDOK = "250 CWD Okay\n";
    char* WELCOME = "220 My FTP Server\n";
    char* USEROK = "331 Please send your password\n";
    char* PASSOK = "230 Password okay\n";
    char* SYST = "215 UNIX FTP Server\n";
    char* FEAT = "211-Features:\r\n MDTM\r\n REST STREAM\r\n SIZE\r\n MLST type*;size*;modify*;perm*;\r\n MLSD\r\n AUTH SSL\r\n AUTH TLS\r\n PROT\r\n PBSZ\r\n UTF8\r\n TVFS\r\n EPSV\r\n EPRT\r\n MFMT\r\n211 End\n";
    char* OPTS = "202 UTF8 OKAY\n";
    char* TYPE = "200 Type set\n";
    char* NOCOMMAND = "202 Command not implemented\n";
    char* DATA_START = "150 Start transferring data on the data channel\n";
    char* DATA_COMPLETED = "226 Successfully sent/recv\n";

    char path[1024];

    int cfd = ((struct THREADPARAM*)arg)->cfd;
    int pasvport = ((struct THREADPARAM*)arg)->port;
    int dfd = -1;

    char buffer[1024];
    send(cfd, WELCOME, strlen(WELCOME), 0);

    while (0 == 0) {
        memset(buffer, 0, sizeof(buffer));
        recv(cfd, buffer, sizeof(buffer), 0);

        if (strncmp(buffer, "LOGIN ", 6) == 0) {

        }
        else if (strncmp(buffer, "PWD", 3) == 0) {     // print current directory
            char rsp[2048];
            memset(rsp, 0, sizeof(rsp));
            sprintf(rsp, "257 \"%s\" is current directory.\n", path);
            send(cfd, rsp, strlen(rsp), 0);
        }
        else if (strncmp(buffer, "PASV", 4) == 0) {
            pasvport = 1024 + (rand() % 32767); 
            int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            struct sockaddr_in saddr;
            struct sockaddr caddr; //Bien ra chua dia chi client noi den
            int clen = sizeof(caddr); //Bien vao + ra chua so byte duoc ghi vao caddr
            saddr.sin_family = AF_INET;
            saddr.sin_port = htons(pasvport);
            saddr.sin_addr.s_addr = 0; //ANY ADDRESS
            bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr));
            listen(sfd, 10);

            char rsp[1024];
            memset(rsp, 0, sizeof(rsp));
            sprintf(rsp, "227 Entering Passive Mode (172,29,202,83,%d,%d)\n", (pasvport & 0xFF00) >> 8, pasvport & 0x00FF);
            send(cfd, rsp, strlen(rsp), 0);

            dfd = accept(sfd, (struct sockaddr*)&caddr, &clen);
        }
        else if (strncmp(buffer, "LS", 2) == 0) {    // list files in current directory
            char* resp = NULL;
            scan(path, &resp);
            send(cfd, DATA_START, strlen(DATA_START), 0);
            send(dfd, resp, strlen(resp), 0);
            close(dfd);
            send(cfd, DATA_COMPLETED, strlen(DATA_COMPLETED), 0);
        }
        else if (strncmp(buffer, "CWD", 3) == 0) {    // change working directory
            char* dir = buffer + 4; //CWD <FOLDER>
            while ( dir[strlen(dir) - 1] == '\r' ||
                    dir[strlen(dir) - 1] == '\n')
                    {
                        dir[strlen(dir) - 1] = 0;
                    }
            sprintf(path + strlen(path), "%s/", dir);
            send(cfd, CWDOK, strlen(CWDOK), 0);
        }
        else if (strncmp(buffer, "CDUP", 4) == 0) {     // go to intermediate upper directory
            if (strlen(path) > 1)
            {
                path[strlen(path) - 1] = 0;
                while (path[strlen(path) - 1] != '/')
                {
                    path[strlen(path) - 1] = 0;
                }
            }
            send(cfd, CDUPOK, strlen(CDUPOK), 0);        
        }
        else if (strncmp(buffer, "DOWNLOAD", 7) == 0) {     // Download a file to local machine
           
        }
        else if (strncmp(buffer, "UPLOAD", 6) == 0) {       // Upload to server
            
        }
        else {
            send(cfd, NOCOMMAND, strlen(NOCOMMAND), 0);    
        }
    }

    close(cfd);
    free(arg); 
    arg = NULL;
    return NULL;
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