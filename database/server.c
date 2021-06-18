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

char activeUser[20] = "";
char activeDB[20] = "";
const char delimiter[10] = " ,=();\n";
const char tempPath[20] = "databases/temp.csv";

int splitString(char splitted[][100], char str[]) {
    int counter=0;
    char *token = strtok(str, delimiter);

    while (token != NULL) {
        strcpy(splitted[counter], token);
        counter++;
        token = strtok(NULL, delimiter);
    }
    return counter;
}

int doHaveAccess(char dbname[]) {
    FILE *fp = fopen("databases/credentials/access.csv", "r");
    char line[100];
    int haveAccess = 0;
    char data[100];
    sprintf(data, "%s,%s", dbname, activeUser);

    while (fscanf(fp, "%s", line) != EOF) {
        if (!strcmp(data, line)) {
            haveAccess = 1;
            break;
        }
    }
    return haveAccess;
}

#include "auth.h"
#include "ddl.h"
#include "dml.h"

void clearBuffer(char* b) {
    for (int i = 0; i < BUFSIZ; i++)
        b[i] = '\0';
}

int dumpDB(char dbname[]) {
    char buffer[BUFSIZ];
    char infoPath[100], tableName[20];
    sprintf(infoPath, "databases/%s/info.csv", dbname);
    FILE *fpInfo = fopen(infoPath, "r");
    while (fgets(tableName, sizeof(tableName), fpInfo) != NULL) {
        char tablePath[100];
        tableName[strcspn(tableName, "\n")] = 0;
        sprintf(tablePath, "databases/%s/%s.csv", dbname, tableName);

        FILE *fpTable = fopen(tablePath, "r");
        FILE *fpTemp = fopen(tempPath, "w");
        char tableLine[100];

        fprintf(fpTemp, "DROP TABLE %s;\n", tableName);
        fprintf(fpTemp, "CREATE TABLE %s (", tableName);
        char colNameSplitted[100][100], colTypeSplitted[100][100];
        fgets(tableLine, sizeof(tableLine), fpTable); // get columns name of table  

        int amount = splitString(colNameSplitted, tableLine);
        fgets(tableLine, sizeof(tableLine), fpTable);
        amount = splitString(colTypeSplitted, tableLine);
        int counter = 0;
        while (counter < amount) {
            if (counter > 0)
                fprintf(fpTemp, ", ");
            fprintf(fpTemp, "%s %s", colNameSplitted[counter], colTypeSplitted[counter]);
            counter++;
        }
        fprintf(fpTemp, ");\n\n");

        while(fgets(tableLine, sizeof(tableLine), fpTable) != NULL) {
            char dataSplitted[100][100];
            fprintf(fpTemp, "INSERT INTO %s (", tableName);
            int amount = splitString(dataSplitted, tableLine);
            counter = 0;
            while (counter < amount) {
                if (counter > 0)
                    fprintf(fpTemp, ", ");
                fprintf(fpTemp, "%s", dataSplitted[counter]);
                counter++;
            }
            fprintf(fpTemp, ");\n");
        }
        fclose(fpTable);
        fclose(fpTemp);
    }
    fclose(fpInfo);
    return 1;
}

int dumpInterface(char* buffer) {
    char query[100];
    strcpy(query, buffer);
    char splitted[100][100];
    int amount = splitString(splitted, query);

    int res = 0;
    if (!strcmp(splitted[0], "DUMP")) {
        char dbname[100];
        strcpy(dbname, splitted[1]);
        if (dumpDB(dbname))
            res = 1;
    }
    return res;
}

void handleQuery(int socketfd) {
    char buffer[BUFSIZ];
    while (1)
    {
        clearBuffer(buffer);
        read(socketfd, buffer, BUFSIZ);
        printf("%s\n", buffer);

        int res = 0;
            
        if (dumpInterface(buffer))
            res = 1;
        else if (authInterface(buffer))
            res = 1;
        else if (ddlInterface(buffer))
            res = 1;
        else if (dmlInterface(buffer))
            res = 1;

        if (res){
            char data[BUFSIZ];
            clearBuffer(data);
            FILE *fpTemp = fopen(tempPath, "r");
            char line[BUFSIZ];
            while (fgets(line, sizeof(line), fpTemp) != NULL) {
                strcat(data, line);
            }
            send(socketfd, data, strlen(data), 0);
            fclose(fpTemp);
        }
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

    if (stat("databases", &st) == -1) {
        mkdir("databases", 0700);
    }
    if (stat("databases/credentials", &st) == -1) {
        mkdir("databases/credentials", 0700);
    }
    FILE* fpUser = fopen("databases/credentials/users.csv", "a+");
    // fprintf(fpUser, "username,password\n");
    fclose(fpUser);
    FILE* fpAccess = fopen("databases/credentials/access.csv", "a+");
    // fprintf(fpAccess, "dbname,username\n");
    fclose(fpAccess);

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