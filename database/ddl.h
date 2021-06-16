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
    fprintf(fpTable, "\n");
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
    char *col;
    int counter = 0;
    col = strtok(headTable, ",");
    // find requested column name
    while(col != NULL) {
        if (!strcmp(columnName,col)) {
            res = 1;
            break;
        }
        col = strtok(NULL, ",");
        counter++;
    }
    // if column name exist, then delete all coresponding value in that column
    if (res) {
        char *tempFileName = "temp.csv";
        FILE *fp = fopen(path, "r");
        FILE *fp_temp = fopen(tempFileName, "w"); 

        char line[100];

        while (fgets(line, sizeof(line), fp) != NULL) {
            char *chunk;
            int chunkCounter = 0;
            chunk = strtok(line, ",");
            while(chunk != NULL) {
                if (chunkCounter != counter) {
                    if (chunkCounter > 0 && !(counter == 0 && chunkCounter == 1))
                        fprintf(fp_temp, ",");
                    fprintf(fp_temp, "%s", chunk);
                }
                chunk = strtok(NULL, ",");
                chunkCounter++;
            }
        }
        fclose(fp);
        fclose(fp_temp);
        remove(path);
        rename(tempFileName, path);
    }
    return res;
}

int ddlInterface(char* buffer) {
    char query[100];
    char *word;
    strcpy(query, buffer);
    word = strtok(query, " ");

    int res = 0;
    if (!strcmp(word, "CREATE")) {
        word = strtok(NULL, " ");
        if (!strcmp(word, "DATABASE")) {
            word = strtok(NULL, " ");
            char dbname[20];
            strcpy(dbname, word);
            if (createDB(dbname)){
                res = 1;
                strcpy(activeDB, dbname);
            }
        }
        else if (!strcmp(word, "TABLE")) {
            word = strtok(NULL, " ");
            char tableName[20];
            strcpy(tableName, word);
            int offset = strlen("CREATE TABLE ") + strlen(tableName) + 1;
            char* colQuery;
            colQuery = query+offset; // get query after table name, inside "()"
            if (colQuery[0] == '(' && colQuery[strlen(colQuery)-1] == ')') {
                colQuery++; // remove first character '('
                colQuery[strlen(colQuery)-1] = '\0'; // remove last character ')'

                char colName[20][20], colType[20][20]; // to store column name and type
                int colCounter = 0, queryIndex = 0, wordIndex;
                // loop every column name and column type
                while (colQuery[queryIndex] != '\0') {
                    wordIndex=0;
                    // get column name
                    while(colQuery[queryIndex] != ' ' && colQuery[queryIndex] != '\0') {
                        colName[colCounter][wordIndex] = colQuery[queryIndex];
                        queryIndex++, wordIndex++;
                    }
                    colName[colCounter][wordIndex] = '\0';
                    queryIndex++, wordIndex=0;
                    // get column type
                    while(colQuery[queryIndex] != ',' && colQuery[queryIndex] != '\0') {
                        colType[colCounter][wordIndex] = colQuery[queryIndex];
                        queryIndex++, wordIndex++;
                    }
                    colType[colCounter][wordIndex] = '\0';
                    queryIndex++;
                    if (colQuery[queryIndex] == ' ')
                        queryIndex++;
                    colCounter++;
                }
                if (createTable(tableName, colName, colType, colCounter))
                    res = 1;
            }
        }
    } 
    else if (!strcmp(word, "DROP")) {
        word = strtok(NULL, " ");
        if (!strcmp(word, "DATABASE")) {
            word = strtok(NULL, " ");
            char dbname[20];
            strcpy(dbname, word);
            if (doHaveAccess(word)){
                if (dropDB(dbname)) {
                    res = 1;
                    strcpy(activeDB, "");
                }
            }
            else {
                printf("You dont have access\n");
            }
            
        }
        else if (!strcmp(word, "TABLE") && strlen(activeDB) > 0) {
            word = strtok(NULL, " ");
            char tableName[20];
            strcpy(tableName, word);
            if (dropTable(tableName)) {
                res = 1;
            }
        }
        else if (!strcmp(word, "COLUMN") && strlen(activeDB) > 0) {
            word = strtok(NULL, " ");
            char columnName[20];
            strcpy(columnName, word);
            word = strtok(NULL, " ");
            if (!strcmp(word, "FROM")) {
                word = strtok(NULL, " ");
                char tableName[20];
                strcpy(tableName, word);
                if (dropColumn(tableName, columnName)) {
                    res = 1;
                }
            }
        }
    }
    return res;
}