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

char active_user[20];

#include "auth.h"
#include "ddl.h"
#include "dml.h"

void clear_buffer(char* b) {
    for (int i = 0; i < BUFSIZ; i++)
        b[i] = '\0';
}

void save_user(char username[], char password[]) {
    FILE *fp_user;
    fp_user = fopen("akun.txt", "a+");

    fprintf(fp_user, "%s,%s\n", username, password);

    printf("save user success\n");
}

void handle_query(int socketfd) {
    char buffer[BUFSIZ];
    while (1)
    {
        read(socketfd, buffer, BUFSIZ);
        printf("%s\n", buffer);

        int res = 0;
        
        char *word;
        word = strtok(buffer, " ");

        if (!strcmp(word, "CREATE")) {
            word = strtok(NULL, " ");
            if (!strcmp(word, "USER")) {
                word = strtok(NULL, " ");
                char username[20], password[20];
                strcpy(username, word);
                word = strtok(NULL, " ");
                if (!strcmp(word, "IDENTIFIED")) {
                    word = strtok(NULL, " ");
                    if (!strcmp(word, "BY")) {
                        word = strtok(NULL, " ");
                        strcpy(password, word);
                        save_user(username, password);
                        res = 1;
                    } 
                }
            }
        } 

        if (res)
            send(socketfd, OK, strlen(OK), 0);
        else 
            send(socketfd, FAIL, strlen(FAIL), 0);
    } 
}

int launch_server() {
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
    int socketfd = launch_server();
    char buffer[BUFSIZ];
    read(socketfd, buffer, BUFSIZ);
    int res = login(socketfd, buffer);
    if (res) 
        send(socketfd, OK, strlen(OK), 0);
    else 
        send(socketfd, FAIL, strlen(FAIL), 0);

    if (res)
        handle_query(socketfd);
    return 0;
}