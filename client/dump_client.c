#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>

#define PORT 8080
#define OK "200"
#define FAIL "100"

int login(int socketfd, char *username, char *password, int root) {
    char data[100], buffer[BUFSIZ];
    sprintf(data, "%s,%s", username, password);
    if (root)
        send(socketfd, "root", 4, 0);
    else 
        send(socketfd, data, strlen(data), 0);

    read(socketfd, buffer, BUFSIZ);
    if (strcmp(buffer, OK))
        return 0;
    return 1;
}

void clearBuffer(char* b) {
    for (int i = 0; i < BUFSIZ; i++)
        b[i] = '\0';
}

int createSocket() {
    struct sockaddr_in address;
    int socketfd, valread;
    struct sockaddr_in serv_addr;
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        exit(0);
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
        exit(0);
  
    if (connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        exit(0);

    return socketfd;
}

int main(int argc, char *argv[]) {
    int socketfd = createSocket();

    if (socketfd == -1)
        exit(0);

    int res, root; // root is 1 if program run by root
    if (geteuid() != 0) {
        root = 0;
        res = login(socketfd, argv[2], argv[4], root);
    }
    else {
        root = 1;
        res = login(socketfd, argv[2], argv[4], root);
    }

    if (!res) {
        printf("Credential not valid. Try again.\n");
        exit(0);
    }

    char query[100], buffer[BUFSIZ];
    sprintf(query, "DUMP %s;", argv[5]); // argv[5] = dbname
    
    send(socketfd, query, strlen(query), 0);
    read(socketfd, buffer, BUFSIZ);
    if (!strcmp(buffer, OK))
        printf("query berhasil\n");
    else 
        printf("query gagal tidak valid\n");

    return 0;
}