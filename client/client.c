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

char activeUser[20];

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

void interface(int socketfd, int root) {
    char buffer[BUFSIZ], query[BUFSIZ];
    printf("yousql> ");
    while(fgets(query, sizeof(query), stdin) != NULL) {
        query[strcspn(query, "\n")] = 0;
        if (strlen(query) == 0) {
            continue;
        }
        if (!strcmp(query, "help")){
            printf("query list:\n\n");

            printf("Admin Only:\n");
            printf("  CREATE USER [username] IDENTIFIED BY [user_password];\n\n");
            printf("  GRANT PERMISSION [database_name] INTO [username];\n\n");

            printf("All authenticate User:\n");
            printf("  USE [database_name];\n\n");
            printf("  CREATE DATABASE [database_name];\n\n");
            printf("  CREATE TABLE [table_name] \n\t([column_name] [data_type], ...);\n\n");
            printf("  DROP [DATABASE | TABLE | COLUMN] \n\t[database_name | table_name | column_name] \n\tFROM [table_name];\n\n");
            printf("  INSERT INTO [table_name] \n\t([value], ...);\n\n");
            printf("  UPDATE [table_name] \n\tSET [column_name]=[value];\n\n");
            printf("  DELETE FROM [table_name];\n\n");
            printf("  SELECT [column_name, ... | *] \n\tFROM [table_name];\n\n");
            printf("  [Command UPDATE, SELECT, DELETE] \n\tWHERE [column_name]=[value];\n\n");
        }
        else if (!strcmp(query, "clear")) {
            system("clear");
        }
        else {
            send(socketfd, query, strlen(query), 0);
            clearBuffer(buffer);
            read(socketfd, buffer, BUFSIZ);
            if (strcmp(buffer, FAIL))
                printf("%s\n", buffer );
            else 
                printf("query not valid\n\n");   
        }
        printf("yousql> ");
    }
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
    
    printf("Welcome to the YouSQL. Commands end with ;\n\n");
    printf("Type 'help' for help. Type 'clear' to clear command line\n\n");

    interface(socketfd, root);

    return 0;
}