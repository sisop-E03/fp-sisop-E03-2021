#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <wait.h>
#include <time.h>

#define PORT 8080
#define OK "200"
#define FAIL "100"

char activeUser[20];
char activeDB[20];

#include "auth.h"
#include "ddl.h"
#include "dml.h"

void clearBuffer(char* b) {
    for (int i = 0; i < BUFSIZ; i++)
        b[i] = '\0';
}

void handleQuery(int socketfd) {
    char buffer[BUFSIZ];
    while (1)
    {
        clearBuffer(buffer);
        read(socketfd, buffer, BUFSIZ);
        printf("%s\n", buffer);
        char query[100];
        strcpy(query, buffer);

        int res = 0;

        if (query[strlen(query)-1] == ';'){
            // remove semicolon in query
            query[strlen(query)-1] = '\n';

            char *word;
            word = strtok(query, " ");

            printf(";\n");

            if (authInterface(query, word))
                res = 1;
            else if (ddlInterface(query, word))
                res = 1;
            else if (dmlInterface(query, word))
                res = 1;
        }

        if (res)
            send(socketfd, OK, strlen(OK), 0);
        else 
            send(socketfd, FAIL, strlen(FAIL), 0);
    } 
}

int launchServer() {
    int server_fd, socketfd, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFSIZ] = {0};

    pthread_t tid[100];
    int connections=0;
      
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        exit(0);
      
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        exit(0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
        exit(0);

    if (listen(server_fd, 3) < 0)
        exit(0);

    struct stat st = {0};

    if (stat("database", &st) == -1) {
        mkdir("database", 0700);
    }

    if ((socketfd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        exit(0);

    return socketfd;
}

int main(int argc, char const *argv[]) {
    int socketfd = launchServer();
    char buffer[BUFSIZ];
    read(socketfd, buffer, BUFSIZ);
    int res = login(socketfd, buffer);
    if (res) 
        send(socketfd, OK, strlen(OK), 0);
    else 
        send(socketfd, FAIL, strlen(FAIL), 0);

    if (res)
        handleQuery(socketfd);
    return 0;
}