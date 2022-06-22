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

void append(char* data, char** presp)
{
    int oldlen = (*presp == NULL ? 0 : strlen(*presp));
    *presp = (char*)realloc(*presp, oldlen + strlen(data) + 1);
    memset(*presp + oldlen, 0, strlen(data) + 1);
    sprintf(*presp + strlen(*presp), "%s", data);
}

// Check client in data base
int checkDataBase(char* userName, char* password) {
    // Write code to authentication user here
    return 1;
}

// Thread to handle with client
void* thread_proc(void* arg) {
    int cfd = *((int*)arg);
}

int main(int argc, char** argv) {
    char userName[1024];
    char password[1024];
    memset(userName, 0, sizeof(userName));
    memset(password, 0, sizeof(password));

    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in saddr;
    struct sockaddr caddr;          //Bien ra chua dia chi client noi den
    int clen = sizeof(caddr);       //Bien vao + ra chua so byte duoc ghi vao caddr
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8080);
    saddr.sin_addr.s_addr = 0;      //ANY ADDRESS

    bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr));

    listen(sfd, 10);
    
    while (1)
    {
        int tmp = accept(sfd, (struct sockaddr*)&caddr, &clen);
        if (tmp >= 0) {

            char buffer[1024 * 16];
            memset(buffer, 0, sizeof(buffer));

            int r = recv(tmp, buffer, sizeof(buffer), 0);

            if(r > 0) {
                printf("server received: {%s}\n", buffer);
                sscanf(buffer, "userName: %s password: %s", userName, password);

                if(checkDataBase(userName, password)){                        //This is to check the database
                    //Handle each client in a separated thread here
                    int* arg = (int*)calloc(1, sizeof(int));
                    *arg = tmp;
                    pthread_t tid = 0;
                    pthread_create(&tid, NULL, thread_proc, arg);
                }
            } 
        }
    }
}