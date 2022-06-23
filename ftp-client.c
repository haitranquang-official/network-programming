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

int main(int argc, char** argv)
{
    char userName[1024];
    char password[1024];
    char *buffer = NULL;

    memset(userName, 0 , sizeof(userName));
    memset(password, 0 , sizeof(password));

    printf("Input username: \n");
    scanf("%s", userName);

    printf("Input password: \n");
    scanf("%s", password);

    append("userName: ", &buffer);
    append(userName, &buffer);
    append(" password: ", &buffer);
    append(password, &buffer);

    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sfd >= 0) {
        struct sockaddr_in saddr;
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(8080);
        saddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        int result = connect(sfd, (struct sockaddr*)&saddr, sizeof(saddr));

        if(result == 0) {
            while(1) {
                send(sfd, buffer, strlen(buffer), 0);

                char reply[1024*20];
                memset(reply, 0, sizeof(reply));            

                result = recv(sfd, reply, sizeof(reply), 0);
                if(result < 0) {
                    break;
                }
            }
        }
        close(sfd);
    }
}