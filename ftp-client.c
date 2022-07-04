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

int sfd = -1;
int dfd = -1;

void append(char* data, char** presp)
{
    int oldlen = (*presp == NULL ? 0 : strlen(*presp));
    *presp = (char*)realloc(*presp, oldlen + strlen(data) + 1);
    memset(*presp + oldlen, 0, strlen(data) + 1);
    sprintf(*presp + strlen(*presp), "%s", data);
}

void signal_handler(int sig) {
    close(sfd);
    close(dfd);
    exit(0); //Terminate the main process
}

void connect_to_server() {
    char username[256];
    char password[256];
    char buffer[1024];

    memset(username, 0 , sizeof(username));
    memset(password, 0 , sizeof(password));
    memset(buffer, 0, sizeof(buffer));

    printf("Input username: \n");
    scanf("%s", username);

    printf("Input password: \n");
    scanf("%s", password);

    sprintf(buffer, "LOGIN %s %s", username, password);

    // đây mới là kết nối tới command port
    sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sfd >= 0) {
        struct sockaddr_in saddr;
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(8080);
        saddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        int result = connect(sfd, (struct sockaddr*)&saddr, sizeof(saddr));

        if(result == 0) {
            // nhận welcome message từ server
            char welcome_message[1024];
            memset(welcome_message, 0, sizeof(welcome_message));
            recv(sfd, welcome_message, sizeof(welcome_message), 0);
            printf("%s", welcome_message);

            // gửi yêu cầu LOGIN lên server
            send(sfd, buffer, strlen(buffer), 0);

            memset(buffer, 0, sizeof(buffer));
            recv(sfd, buffer, sizeof(buffer), 0);
            printf("%s", buffer);

            // gọi lệnh PASV để thiết lập data port theo kiểu passive
        }
    }
}

int main(int argc, char** argv)
{

    connect_to_server();

    // char buffer[1024];
    // memset(buffer, 0, sizeof(buffer));

    // while(1) {
    //     send(sfd, buffer, strlen(buffer), 0);

    //     char reply[1024*20];
    //     memset(reply, 0, sizeof(reply));            

    //     int result = recv(sfd, reply, sizeof(reply), 0);
    //     if(result < 0) {
    //         break;
    //     }
    // }

    close(sfd);

    return 0;
}