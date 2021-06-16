int createDB(char dbname[]) {
    int res = 0;

    // create folder if doesn't exist
    struct stat st = {0};
    if (stat("FILES", &st) == -1) {
        mkdir("FILES", 0700);
        res = 1;
    }
    return res;
}

int dropDB(char dbname[]) {
    int res = 0;

    // not implemented
    return res;
}

int dropColumn(char columnName[]) {
    int res = 0;

    // not implemented
    return res;
}

int dropTable(char tableName[]) {
    int res = 0;

    // not implemented
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
            word = strtok(NULL, " ");
            if (createDB(dbname)){
                res = 1;
                strcpy(activeDB, dbname);
            }
        }
        else if (!strcmp(word, "TABLE")) {
            word = strtok(NULL, " ");
            // not implemented
        }
    } 
    else if (!strcmp(word, "DROP")) {
        word = strtok(NULL, " ");
        if (!strcmp(word, "DATABASE")) {
            word = strtok(NULL, " ");
            char dbname[20];
            strcpy(dbname, word);
            if (dropDB(dbname)) {
                res = 1;
                strcpy(activeDB, "");
            }
        }
        else if (!strcmp(word, "TABLE")) {
            word = strtok(NULL, " ");
            char tableName[20];
            strcpy(tableName, word);
            if (dropTable(tableName)) {
                res = 1;
            }
        }
        else if (!strcmp(word, "COLUMN")) {
            word = strtok(NULL, " ");
            char columnName[20];
            strcpy(columnName, word);
            if (dropColumn(columnName)) {
                res = 1;
            }
        }
    }
    return res;
}