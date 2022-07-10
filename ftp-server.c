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
#include "./login/login.h"
#include "./database/user.h"
#include "./scan_dir/scan_dir.h"
#include "./file_transfer/file_transfer.h"

pthread_t *tid = NULL;
int count = 0;


unsigned short PASVPORT = 10000;

struct THREADPARAM
{
    int cfd;
    int port;
};

void signal_handler(int sig) {
    exit(0); //Terminate the main process
}

// Thread to handle with client
void *thread_proc(void *arg)
{
    char* PORTOK = "250 PORT Okay\n";
    char* CDUPOK = "250 CDUP Okay\n";
    char* CWDOK = "250 CWD Okay\n";
    char* WELCOME = "220 My FTP Server\n";
    char* LOGIN_OK = "911 Login successfully\n";
    char* LOGIN_FAILED = "430 Invalid username or password\n";
    char* SYST = "215 UNIX FTP Server\n";
    char* FEAT = "211-Features:\r\n MDTM\r\n REST STREAM\r\n SIZE\r\n MLST type*;size*;modify*;perm*;\r\n MLSD\r\n AUTH SSL\r\n AUTH TLS\r\n PROT\r\n PBSZ\r\n UTF8\r\n TVFS\r\n EPSV\r\n EPRT\r\n MFMT\r\n211 End\n";
    char* OPTS = "202 UTF8 OKAY\n";
    char* TYPE = "200 Type set\n";
    char* NOCOMMAND = "202 Command not implemented\n";
    char* DATA_START = "150 Start transferring data on the data channel\n";
    char* DATA_COMPLETED = "226 Successfully sent/recv\n";

    char path[1024];
    memset(path, 0, sizeof(path));
    strcpy(path,"/");

    int cfd = ((struct THREADPARAM*)arg)->cfd;
    int pasvport = ((struct THREADPARAM*)arg)->port;
    int dfd = -1;
    int userId = -1;

    char buffer[1024];
    send(cfd, WELCOME, strlen(WELCOME), 0);

    while (0 == 0) {
        memset(buffer, 0, sizeof(buffer));
        recv(cfd, buffer, sizeof(buffer), 0);

        if (strncmp(buffer, "LOGIN ", 6) == 0) {
            char username[256], password[256];
            sscanf(buffer, "LOGIN %s %s", username, password);

            userId = login(username, password);

            if(userId < 0) {
                send(cfd, LOGIN_FAILED, strlen(LOGIN_FAILED), 0);
            }
            else {
                strcpy(path, "/");
                send(cfd, LOGIN_OK, strlen(LOGIN_OK), 0);
            }
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
            socklen_t clen = sizeof(caddr); //Bien vao + ra chua so byte duoc ghi vao caddr
            saddr.sin_family = AF_INET;
            saddr.sin_port = htons(pasvport);
            saddr.sin_addr.s_addr = 0; //ANY ADDRESS
            bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr));
            listen(sfd, 10);

            char rsp[1024];
            memset(rsp, 0, sizeof(rsp));
            sprintf(rsp, "227 Entering Passive Mode (127,0,0,1,%d,%d)\n", (pasvport & 0xFF00) >> 8, pasvport & 0x00FF);
            send(cfd, rsp, strlen(rsp), 0);

            dfd = accept(sfd, (struct sockaddr*)&caddr, &clen);
        }
        else if (strncmp(buffer, "LS", 2) == 0) {    // list files in current directory
            char* resp = NULL;

            char full_path[2048];
            memset(full_path, 0, sizeof(full_path));

            sprintf(full_path, "%s%s", find_home_dir_by_user(userId), path);

            scan(full_path, &resp);

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
                if(strlen(path) > 1 && path[strlen(path) - 1] == '/') {
                    path[strlen(path) - 1] = 0;
                }
            }
            send(cfd, CDUPOK, strlen(CDUPOK), 0);        
        }
        else if (strncmp(buffer, "DOWNLOAD", 8) == 0) {     // Download a file to local machine
            char file_path[1024];
            memset(file_path, 0, sizeof(file_path));
            sscanf(buffer, "DOWNLOAD %s", file_path);

            char full_path[2048];
            memset(full_path, 0, sizeof(full_path));

            strcpy(full_path, path);
            strcpy(full_path + strlen(full_path), file_path);

            download(cfd, dfd, userId, full_path);
        }
        else if (strncmp(buffer, "UPLOAD", 6) == 0) {       // Upload to server
            char filename[256];
            memset(filename, 0, sizeof(filename));
            sscanf(buffer, "UPLOAD %s", filename);

            char full_path[2048];
            memset(full_path, 0, sizeof(full_path));

            strcpy(full_path, path);

            upload(cfd, dfd, userId, filename, full_path);
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
    srand(time(NULL));
    signal(SIGINT, signal_handler);

    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in saddr;
    struct sockaddr caddr; //Bien ra chua dia chi client noi den
    socklen_t clen = sizeof(caddr); //Bien vao + ra chua so byte duoc ghi vao caddr
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8080);
    saddr.sin_addr.s_addr = 0; //ANY ADDRESS

    bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr));

    listen(sfd, 10);

    while (0 == 0) {
        int tmp = accept(sfd, (struct sockaddr*)&caddr, &clen);
        if (tmp >= 0)
        {
            //Handle chat message in a separated thread here
            struct THREADPARAM* arg = (struct THREADPARAM*)calloc(1, sizeof(struct THREADPARAM));
            arg->cfd = tmp;
            arg->port = PASVPORT++;
            pthread_t tid = 0;
            pthread_create(&tid, NULL, thread_proc, arg);
        }
    }
    return 0;
}