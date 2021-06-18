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
#include <syslog.h>

#define PORT 8080
#define OK "200"
#define FAIL "100"

char activeUser[20] = "";
char activeDB[20] = "";
const char delimiter[10] = " ,=();\n";
const char tempPath[20] = "databases/temp.csv";
const char logPath[20] = "query.log";

/* Utils Section */
void createLog(char query[]) {
    FILE *fpLog = fopen(logPath, "a+");
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    
    int year = tm.tm_year+1900;
    int month = tm.tm_mon+1;
    int day = tm.tm_mday;
    int hour = tm.tm_hour;
    int min = tm.tm_min;
    int sec = tm.tm_sec;

    fprintf(fpLog, "%d:%d:%d %d:%d:%d:%s:%s\n", year, month, day, hour, min, sec, activeUser, query);
    fclose(fpLog);
}

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

void clearBuffer(char* b) {
    for (int i = 0; i < BUFSIZ; i++)
        b[i] = '\0';
}

/* Auth Section */
int login(int socketfd, char *data) {
    if (!strcmp(data, "root")){
        strcpy(activeUser, data);
        return 1;
    }

    FILE *fp = fopen("databases/credentials/users.csv", "r");
    char line[100];
    int is_exist = 0;

    while (fscanf(fp, "%s", line) != EOF) {
        if (strcmp(data, line) == 0) {
            char splitted[100][100];
            int amount = splitString(splitted, data);
            strcpy(activeUser, splitted[0]);
            is_exist = 1;
            break;
        }
    }
    if (is_exist) 
        return 1;
    return 0;
}

void createUser(char username[], char password[]) {
    FILE *fpUser;
    fpUser = fopen("databases/credentials/users.csv", "a+");

    fprintf(fpUser, "%s,%s\n", username, password);
    fclose(fpUser);
    
    FILE *fpTemp = fopen(tempPath, "w");
    fprintf(fpTemp, "User created\n");
    fclose(fpTemp);
}

void giveAccess(char dbname[], char username[]) {
    FILE *fpAccess;
    fpAccess = fopen("databases/credentials/access.csv", "a+");

    fprintf(fpAccess, "%s,%s\n", dbname, username);
    fclose(fpAccess);
    FILE *fpTemp = fopen(tempPath, "w");
    fprintf(fpTemp, "Database access given\n");
    fclose(fpTemp);
}

int changeActiveDB(char dbname[]) {
    if (doHaveAccess(dbname)){
        strcpy(activeDB, dbname);
        FILE *fpTemp = fopen(tempPath, "w");
        fprintf(fpTemp, "Active database changed\n");
        fclose(fpTemp);
    }
    else {
        FILE *fpTemp = fopen(tempPath, "w");
        fprintf(fpTemp, "You don't have access to this database\n");
        fclose(fpTemp);
    }
    return 1;
}

int authInterface(char* buffer) {
    char query[100];
    char *word;
    strcpy(query, buffer);
    char splitted[100][100];
    int amount = splitString(splitted, query);

    int res = 0;
    if (!strcmp(splitted[0], "CREATE") && !strcmp(activeUser, "root")) {
        if (!strcmp(splitted[1], "USER")) {
            char username[20], password[20];
            strcpy(username, splitted[2]);
            if (!strcmp(splitted[3], "IDENTIFIED")) {
                if (!strcmp(splitted[4], "BY")) {
                    strcpy(password, splitted[5]);
                    createUser(username, password);
                    res = 1;
                } 
            }
        }
    } 
    else if (!strcmp(splitted[0], "USE")) {
        if (changeActiveDB(splitted[1]));
            res = 1;
    }
    else if (!strcmp(splitted[0], "GRANT") && !strcmp(activeUser, "root")) {
        if (!strcmp(splitted[1], "PERMISSION")) {
            char dbname[20], username[20];
            strcpy(dbname, splitted[2]);
            if (!strcmp(splitted[3], "INTO")) {
                strcpy(username, splitted[4]);
                giveAccess(dbname, username);
                res = 1;
            }
        }
    }
    return res;
}

/* DDL Section */
int createDB(char dbname[]) {
    int res = 0;
    char path[40];
    sprintf(path, "databases/%s", dbname);

    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mkdir(path, 0700);
        FILE *fpAccess = fopen("databases/credentials/access.csv", "a+");
        fprintf(fpAccess, "%s,%s\n", dbname, activeUser);
        fclose(fpAccess);
        res = 1;
    }
    FILE *fpTemp = fopen(tempPath, "w");
    fprintf(fpTemp, "Database created\n");
    fclose(fpTemp);
    return res;
}

int createTable(char tableName[], char colName[][20], char colType[][20], int colAmount) {
    int res = 0;
    char path[100];
    sprintf(path, "databases/%s/%s.csv", activeDB, tableName);
    FILE *fpTable = fopen(path, "a+");
    fprintf(fpTable, "%s", colName[0]);
    for (int i=1; i<colAmount; i++) {
        fprintf(fpTable, ",%s",colName[i]);
    }
    fprintf(fpTable, "\n%s", colType[0]);
    for (int i=1; i<colAmount; i++) {
        fprintf(fpTable, ",%s", colType[i]);
    }
    fprintf(fpTable, "\n");

    sprintf(path, "databases/%s/info.csv", activeDB);
    FILE *fpInfo = fopen(path, "a+");
    fprintf(fpInfo, "%s\n", tableName);
    fclose(fpInfo);
    fclose(fpTable);
    FILE *fpTemp = fopen(tempPath, "w");
    fprintf(fpTemp, "Table created\n");
    fclose(fpTemp);
    return 1;
}

int dropDB(char dbname[]) {
    int res = 0;
    char cmd[100];
    sprintf(cmd, "rm -r databases/%s", dbname);
    system(cmd);
    FILE *fpTemp = fopen(tempPath, "w");
    fprintf(fpTemp, "Database dropped\n");
    fclose(fpTemp);
    return 1;
}

int dropTable(char tableName[]) {
    int res = 0;
    char cmd[100];
    sprintf(cmd, "rm -r databases/%s/%s.csv", activeDB, tableName);
    system(cmd);
    FILE *fpTemp = fopen(tempPath, "w");
    fprintf(fpTemp, "Table dropped\n");
    fclose(fpTemp);
    return 1;
}

int dropColumn(char tableName[], char columnName[]) {
    int res = 0;
    char path[100];
    sprintf(path, "databases/%s/%s.csv", activeDB, tableName);

    FILE *fpTable = fopen(path, "r");
    char headTable[100];
    fscanf(fpTable, "%s", headTable);
    fclose(fpTable);
    int counter = 0;

    char columnNames[100][100];
    int colAmount = splitString(columnNames, headTable);
    while(counter < colAmount) {
        if (!strcmp(columnNames[counter], columnName)) {
            res = 1;
            break;
        }
        counter++;
    }
    // if column name exist, then delete all coresponding value in that column
    if (res) {
        char *tempFileName = "temp.csv";
        FILE *fpTable = fopen(path, "r");
        FILE *fpTemp = fopen(tempFileName, "w"); 

        char line[100];
        while (fgets(line, sizeof(line), fpTable) != NULL) {
            char columnDatas[100][100];
            colAmount = splitString(columnDatas, line);
            int chunkCounter = 0;
            while (chunkCounter < colAmount) {
                if (chunkCounter != counter) {
                    if (chunkCounter > 0 && !(counter == 0 && chunkCounter == 1))
                        fprintf(fpTemp, ",");
                    fprintf(fpTemp, "%s", columnDatas[chunkCounter]);
                }
                chunkCounter++;
            }
            fprintf(fpTemp, "\n");
        }
        fclose(fpTable);
        fclose(fpTemp);
        remove(path);
        rename(tempFileName, path);
    }
    FILE *fpTemp = fopen(tempPath, "w");
    fprintf(fpTemp, "Column Dropped\n");
    fclose(fpTemp);
    return res;
}

int ddlInterface(char* buffer) {
    char query[100];
    strcpy(query, buffer);
    char splitted[100][100];
    int amount = splitString(splitted, query);

    int res = 0;
    if (!strcmp(splitted[0], "CREATE")) {
        if (!strcmp(splitted[1], "DATABASE")) {
            char dbname[20];
            strcpy(dbname, splitted[2]);
            if (createDB(dbname)){
                res = 1;
                strcpy(activeDB, dbname);
            }
        }
        else if (!strcmp(splitted[1], "TABLE")) {
            char tableName[20];
            strcpy(tableName, splitted[2]);
            int splitCounter = 3, colCounter=0;
            char colName[20][20], colType[20][20];
            while (splitCounter < amount) {
                strcpy(colName[colCounter], splitted[splitCounter++]);
                strcpy(colType[colCounter], splitted[splitCounter++]);
                colCounter++;
            }
            if (createTable(tableName, colName, colType, colCounter))
                res = 1;
        }
    } 
    else if (!strcmp(splitted[0], "DROP")) {
        if (!strcmp(splitted[1], "DATABASE")) {
            char dbname[20];
            strcpy(dbname, splitted[2]);
            if (doHaveAccess(dbname)){
                if (dropDB(dbname)) {
                    res = 1;
                    strcpy(activeDB, "");
                }
            }
            else {
                printf("You dont have access\n");
            }
            
        }
        else if (!strcmp(splitted[1], "TABLE") && strlen(activeDB) > 0) {
            char tableName[20];
            strcpy(tableName, splitted[2]);
            if (dropTable(tableName)) {
                res = 1;
            }
        }
        else if (!strcmp(splitted[1], "COLUMN") && strlen(activeDB) > 0) {
            char columnName[20];
            strcpy(columnName, splitted[2]);
            if (!strcmp(splitted[3], "FROM")) {
                char tableName[20];
                strcpy(tableName, splitted[4]);
                if (dropColumn(tableName, columnName)) {
                    res = 1;
                }
            }
        }
    }
    return res;
}

/* DML Section */
int insertData(char tableName[], char data[][20], int colAmount)
{
    char path[100];
    sprintf(path, "databases/%s/%s.csv", activeDB, tableName);
    FILE *fpTable = fopen(path, "a+");
    int counter = 0;
    while (counter < colAmount)
    {
        if (counter > 0)
            fprintf(fpTable, ",");
        fprintf(fpTable, "%s", data[counter++]);
    }
    fprintf(fpTable, "\n");
    fclose(fpTable);
    FILE *fpTemp = fopen(tempPath, "w");
    fprintf(fpTemp, "1 row data inserted\n");
    fclose(fpTemp);
    return 1;
}

int updateData(char tableName[], char column[], char newValue[],
               char condColumn[], char condValue[])
{
    int res = 0;
    int whereCond = 0;

    char path[100];
    sprintf(path, "databases/%s/%s.csv", activeDB, tableName);
    FILE *fpTable = fopen(path, "r");
    FILE *fpTemp;

    char headTable[100];
    fgets(headTable, sizeof(headTable), fpTable);

    char columnNames[100][100];
    char tempHeadTable[100];

    strcpy(tempHeadTable, headTable);
    int colAmount = splitString(columnNames, tempHeadTable);

    if (strcmp(condValue, ""))
        whereCond = 1;

    int counter = 0;
    int colIndex = -1;
    int condColIndex = -1;
    while (counter < colAmount)
    {
        if (!strcmp(columnNames[counter], column))
        {
            res = 1;
            colIndex = counter;
        }

        if (!strcmp(columnNames[counter], condColumn))
            condColIndex = counter;

        if (res && (!whereCond || (whereCond && condColIndex > -1)))
            break;

        counter++;
    }
    int rowUpdatedCounter = 0;
    if (res)
    {
        char *tempFileName = "temp.csv";
        fpTemp = fopen(tempFileName, "w");
        fputs(headTable, fpTemp);

        char line[100];
        fgets(line, sizeof(line), fpTable);
        fputs(line, fpTemp);

        while (fgets(line, sizeof(line), fpTable) != NULL)
        {
            char columnDatas[100][100];
            char tempLine[100];
            strcpy(tempLine, line);
            colAmount = splitString(columnDatas, tempLine);

            if (whereCond && strcmp(condValue, columnDatas[condColIndex]))
            {
                fputs(line, fpTemp);
                continue;
            }

            int chunkCounter = 0;
            while (chunkCounter < colAmount)
            {
                if (chunkCounter > 0)
                    fprintf(fpTemp, ",");

                if (chunkCounter != colIndex)
                {
                    fprintf(fpTemp, "%s", columnDatas[chunkCounter]);
                }
                else
                {
                    fprintf(fpTemp, "%s", newValue);
                }

                chunkCounter++;
            }
            rowUpdatedCounter++;
            fprintf(fpTemp, "\n");
        }

        fclose(fpTable);
        fclose(fpTemp);

        remove(path);
        rename(tempFileName, path);
    }
    fpTemp = fopen(tempPath, "w");
    fprintf(fpTemp, "%d row data updated\n", rowUpdatedCounter);
    fclose(fpTemp);
    return res;
}

int deleteData(char tableName[], char condColumn[], char condValue[])
{
    int res = 0;
    int whereCond = 0;

    char path[100];
    sprintf(path, "databases/%s/%s.csv", activeDB, tableName);
    FILE *fpTable = fopen(path, "r");

    char headTable[100];
    fgets(headTable, sizeof(headTable), fpTable);

    char columnNames[100][100];
    char tempHeadTable[100];

    strcpy(tempHeadTable, headTable);
    int colAmount = splitString(columnNames, tempHeadTable);

    if (strcmp(condColumn, ""))
        whereCond = 1;

    int counter = 0;
    if (whereCond)
    {
        while (counter < colAmount)
        {
            if (!strcmp(columnNames[counter], condColumn))
            {
                break;
            }

            counter++;
        }
    }

    char *tempFileName = "temp.csv";
    FILE *fpTemp = fopen(tempFileName, "w");
    fputs(headTable, fpTemp);

    char line[100];
    fgets(line, sizeof(line), fpTable);
    fputs(line, fpTemp);

    int rowDeletedCounter = 0;
    if (whereCond)
    {
        while (fgets(line, sizeof(line), fpTable) != NULL)
        {
            char columnDatas[100][100];
            char tempLine[100];
            strcpy(tempLine, line);
            colAmount = splitString(columnDatas, tempLine);

            if (strcmp(condValue, columnDatas[counter]))
                fputs(line, fpTemp);
            else
                rowDeletedCounter++;
        }
    }

    fclose(fpTable);
    fclose(fpTemp);

    remove(path);
    rename(tempFileName, path);
    res = 1;

    fpTemp = fopen(tempPath, "w");
    fprintf(fpTemp, "%d row deleted\n", rowDeletedCounter);
    fclose(fpTemp);

    return res;
}

int selectData(char tableName[], char columns[][100], char condColumn[], char condValue[])
{
    int res = 0;
    int whereCond = 0;

    char path[100];
    sprintf(path, "databases/%s/%s.csv", activeDB, tableName);
    FILE *fpTable = fopen(path, "r");
    FILE *fpTemp = fopen(tempPath, "w");

    char headTable[100];
    fgets(headTable, sizeof(headTable), fpTable);

    char columnNames[100][100];

    int colAmount = splitString(columnNames, headTable);

    if (strcmp(condValue, ""))
        whereCond = 1;

    int colIndex[100];
    memset(colIndex, -1, sizeof(colIndex));
    int condColIndex = -1;
    int counter = 0;

    if (!strcmp(columns[0], "*"))
    {
        int i = 0;
        while (i < colAmount)
        {
            colIndex[counter++] = i;
            fprintf(fpTemp, "%s\t", columnNames[i]);

            if (!strcmp(columnNames[i], condColumn))
                condColIndex = i;

            i++;
        }
    }
    else
    {
        int i = 0;
        while (strcmp(columns[i], ""))
        {
            int j = 0;
            while (j < colAmount)
            {

                if (!strcmp(columnNames[j], columns[i]))
                {
                    colIndex[counter++] = j;
                    fprintf(fpTemp, "%s\t", columnNames[j]);
                }

                if (whereCond && condColIndex < 0 && !strcmp(columnNames[j], condColumn))
                    condColIndex = j;

                j++;
            }

            i++;
        }
    }

    fprintf(fpTemp, "\n");

    char line[100];
    fgets(line, sizeof(line), fpTable);
    while (fgets(line, sizeof(line), fpTable) != NULL)
    {
        char columnDatas[100][100];
        colAmount = splitString(columnDatas, line);

        int show = 0;
        if (!whereCond || (whereCond && !strcmp(condValue, columnDatas[condColIndex])))
        {
            int i = 0;
            while (colIndex[i] != -1)
            {
                int chunkCounter = 0;
                while (chunkCounter < colAmount)
                {
                    if (chunkCounter == colIndex[i])
                    {
                        int lastChar = strlen(columnDatas[chunkCounter]) - 1;
                        if (columnDatas[chunkCounter][lastChar] == '\'')
                            columnDatas[chunkCounter][lastChar] = '\0';

                        if (columnDatas[chunkCounter][0] == '\'')
                            fprintf(fpTemp, "%s\t", columnDatas[chunkCounter] + 1);
                        else
                            fprintf(fpTemp, "%s\t", columnDatas[chunkCounter]);
                        break;
                    }

                    chunkCounter++;
                }
                
                i++;
            }
            fprintf(fpTemp, "\n");
        }
    }
    fclose(fpTemp);
    fclose(fpTable);

    return 1;
}

int dmlInterface(char *buffer)
{
    char query[100];
    strcpy(query, buffer);
    char splitted[100][100];
    memset(splitted, '\0', sizeof(splitted));
    int amount = splitString(splitted, query);

    int res = 0;
    if (!strcmp(splitted[0], "INSERT"))
    {
        if (!strcmp(splitted[1], "INTO"))
        {
            char tableName[20];
            strcpy(tableName, splitted[2]);
            int splitCounter = 3, colCounter = 0;
            char data[100][20];
            while (splitCounter < amount)
            {
                strcpy(data[colCounter++], splitted[splitCounter++]);
            }
            if (insertData(tableName, data, amount - 3))
                res = 1;
        }
    }
    else if (!strcmp(splitted[0], "UPDATE"))
    {
        char tableName[20];
        strcpy(tableName, splitted[1]);
        if (!strcmp(splitted[2], "SET"))
        {
            if (updateData(tableName, splitted[3], splitted[4], splitted[6], splitted[7]))
                res = 1;
        }
    }
    else if (!strcmp(splitted[0], "DELETE"))
    {
        if (!strcmp(splitted[1], "FROM"))
        {
            char tableName[20];
            strcpy(tableName, splitted[2]);

            if (deleteData(tableName, splitted[4], splitted[5]))
                res = 1;
        }
    }
    else if (!strcmp(splitted[0], "SELECT"))
    {
        int i = 1;
        char tableName[100];
        char columns[100][100];
        memset(columns, '\0', sizeof(columns));
        char condColumn[100];
        char condValue[100];
        while (strcmp(splitted[i], "") && strcmp(splitted[i], "FROM"))
        {
            strcpy(columns[i - 1], splitted[i]);
            i++;
        }

        strcpy(tableName, splitted[i + 1]);
        strcpy(condColumn, splitted[i + 3]);
        strcpy(condValue, splitted[i + 4]);

        if (selectData(tableName, columns, condColumn, condValue))
            res = 1;
    }
    return res;
}

/* Dump Section */
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
        fprintf(fpTemp, "USE %s\n\n", dbname);

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


/* Interface Section */
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
            createLog(buffer);

            char data[BUFSIZ];
            clearBuffer(data);
            FILE *fpTemp = fopen(tempPath, "r");
            char line[BUFSIZ];
            while (fgets(line, sizeof(line), fpTemp) != NULL) {
                strcat(data, line);
            }
            send(socketfd, data, strlen(data), 0);
            fclose(fpTemp);
            remove(tempPath);
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
    fclose(fpUser);
    FILE* fpAccess = fopen("databases/credentials/access.csv", "a+");
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