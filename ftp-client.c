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

char *strrev(char *str)
{
    if (!str || ! *str)
        return str;

    int i = strlen(str) - 1, j = 0;

    char ch;
    while (i > j)
    {
        ch = str[i];
        str[i] = str[j];
        str[j] = ch;
        i--;
        j++;
    }
    return str;
}

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
        }
    }
}

void init_data_connection() {
    char* request = "PASV";

    // gọi lệnh PASV để lấy data connection port
    send(sfd, request, strlen(request), 0);

    char response[1024];

    recv(sfd, response, sizeof(response), 0);

    printf("%s", response);

    int pre, post;
    sscanf(response, "227 Entering Passive Mode (127,0,0,1,%d,%d)", &pre, &post);

    int port = pre * 256 + post;

    dfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (dfd >= 0) {
        struct sockaddr_in saddr;
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(port);
        saddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        int result = connect(dfd, (struct sockaddr*)&saddr, sizeof(saddr));

        if(result == -1) {
            dfd = -1;
        }
    }
}

void respond_to_server(int fd) {
    char* ok = "OK\n";
    send(fd, ok, strlen(ok), 0);
}

void process() {
    char request[256];
    char response[1024];

    memset(request, 0, sizeof(request));
    memset(response, 0, sizeof(response));

    while(1) {
        fgets(request, sizeof(request), stdin);

        if(strlen(request) == 0) {
            continue;
        }

        if(strncmp(request, "DOWNLOAD", 8) == 0) {
            char filename[256];
            memset(filename, 0, sizeof(filename));

            sscanf(request, "DOWNLOAD %s", filename);

            init_data_connection();

            // gửi command tới server
            send(sfd, request, strlen(request), 0);

            // nhận response ở command port
            memset(response, 0, sizeof(response));
            recv(sfd, response, sizeof(response), 0);

            printf("%s", response);
            respond_to_server(sfd);

            // nhận về size của file
            memset(response, 0, sizeof(response));
            recv(dfd, response, sizeof(response), 0);
            respond_to_server(dfd);
            
            int size;
            sscanf(response, "Content Length: %d", &size);

            char* data = (char*) calloc(size, 1);

            int received = 0;
            // nhận response ở data port
            while(received < size) {
                received += recv(dfd, data + received, size - received, 0);
            }

            respond_to_server(dfd);

            char path[512];
            memset(path, 0, sizeof(path));

            sprintf(path, "/home/hieutran29/Desktop/%s", filename);
            
            FILE* file = fopen(path, "wb");
            fwrite(data, sizeof(char), received, file);
            fclose(file);

            free(data);

            close(dfd);

            memset(response, 0, sizeof(response));
            recv(sfd, response, sizeof(response), 0);
            printf("%s", response);
            respond_to_server(sfd);
        }
        else if(strncmp(request, "UPLOAD", 6) == 0) {
            char response[1024];
            
            init_data_connection();

            char filepath[512];
            memset(filepath, 0, sizeof(filepath));
            sscanf(request, "UPLOAD %s", filepath);

            char filename[512];
            memset(filename, 0, sizeof(filename));
            for(int i = strlen(filepath) - 1; i > 0 && filepath[i] != '/'; i--) {
                sprintf(filename + strlen(filename), "%c", filepath[i]);
            }

            char real_request[1024];
            sprintf(real_request, "%s%s", "UPLOAD ", strrev(filename));

            send(sfd, real_request, strlen(real_request), 0);

            memset(response, 0, sizeof(response));
            recv(sfd, response, sizeof(response), 0);
            printf("%s", response);
            respond_to_server(sfd);

            FILE* file = fopen(filepath, "rb");
                
            fseek(file, 0, SEEK_END);
            int size =  ftell(file);
            fseek(file, 0, SEEK_SET);

            char* data = (char*) calloc(size, 1);
            fread(data, 1, size, file);

            int sent = 0;
            while (sent < size) {
                sent += send(dfd, data + sent, size - sent, 0);
            }

            free(data);
            fclose(file);

            close(dfd);

            memset(response, 0, sizeof(response));
            recv(sfd, response, sizeof(response), 0);
            printf("%s", response);
        }
        else if(strncmp(request, "QUIT", 4) == 0) {
            break;
        }
    }
}

int main(int argc, char** argv)
{
    signal(SIGINT, signal_handler);

    // initialize command connection
    connect_to_server();

    process();

    close(sfd);

    return 0;
}