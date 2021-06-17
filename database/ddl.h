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
    return 1;
}

int dropDB(char dbname[]) {
    int res = 0;
    char cmd[100];
    sprintf(cmd, "rm -r databases/%s", dbname);

    system(cmd);
    return 1;
}

int dropTable(char tableName[]) {
    int res = 0;
    char cmd[100];
    sprintf(cmd, "rm -r databases/%s/%s.csv", activeDB, tableName);

    system(cmd);
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
            while (counter < colAmount) {
                if (chunkCounter != counter) {
                    if (chunkCounter > 0 && !(counter == 0 && chunkCounter == 1))
                        fprintf(fpTemp, ",");
                    fprintf(fpTemp, "%s", columnDatas[chunkCounter]);
                }
                chunkCounter++;
            }
        }
        fclose(fpTable);
        fclose(fpTemp);
        remove(path);
        rename(tempFileName, path);
    }
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