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

#include "auth.h"
#include "ddl.h"
#include "dml.h"

void clear_buffer(char* b) {
    for (int i = 0; i < BUFSIZ; i++)
        b[i] = '\0';
}

int create_socket() {
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
    int socketfd = create_socket();

    if (socketfd == -1)
        exit(0);

    int res;
    if (geteuid() != 0) {
        res = login(socketfd, argv[2], argv[4]);
    } 
    else {
        res = 1;
    }

    if (!res) {
        printf("Credential not valid. Try again.\n");
        exit(0);
    }
    
    printf("Login berhasil\n");

    return 0;
}