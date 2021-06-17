int insertData(char tableName[], char data[][20], int colAmount) {
    char path[100];
    sprintf(path, "databases/%s/%s.csv", activeDB, tableName);
    FILE *fpTable = fopen(path, "a+");
    int counter = 0;
    while (counter < colAmount) {
        if (counter > 0)
            fprintf(fpTable, ",");
        fprintf(fpTable, "%s", data[counter++]);
    }
    fprintf(fpTable, "\n");
    fclose(fpTable);
}

int dmlInterface(char* buffer) {
    char query[100];
    strcpy(query, buffer);
    char splitted[100][100];
    int amount = splitString(splitted, query);
    
    int res = 0;
    if (!strcmp(splitted[0], "INSERT")) {
        if (!strcmp(splitted[1], "INTO")) {
            char tableName[20];
            strcpy(tableName, splitted[2]);
            int splitCounter = 3, colCounter = 0; 
            char data[100][20];
            while (splitCounter < amount) {
                strcpy(data[colCounter++], splitted[splitCounter++]);
            }
            if (insertData(tableName, data, amount-3));
                res = 1;
        }
    } 
    else if (!strcmp(splitted[0], "UPDATE")) {
        char tableName[20];
        strcpy(tableName, splitted[1]);
        if (!strcmp(splitted[2], "SET")) {
            // not implemented

        }
    }
    else if (!strcmp(splitted[0], "DELETE")) {
        if (!strcmp(splitted[1], "FROM")) {
            char tableName[20];
            strcpy(tableName, splitted[2]);
            // not impelemented
        }
    }
    else if (!strcmp(splitted[0], "SELECT")) {
        // not implemented
    }
    return res;
}