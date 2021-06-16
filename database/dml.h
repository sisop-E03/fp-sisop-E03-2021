int dmlInterface(char* query, char* word) {
    int res = 0;
    if (!strcmp(word, "INSERT")) {
        word = strtok(NULL, " ");
        if (!strcmp(word, "INTO")) {
            word = strtok(NULL, " ");
            char tableName[20];
            strcpy(tableName, word);
            // not implemented
        }
    } 
    else if (!strcmp(word, "UPDATE")) {
        word = strtok(NULL, " ");
        char tableName[20];
        strcpy(tableName, word);
        word = strtok(NULL, " ");
        if (!strcmp(word, "SET")) {
            word = strtok(NULL, " ");
            // not implemented
        }
    }
    else if (!strcmp(word, "DELETE")) {
        word = strtok(NULL, " ");
        if (!strcmp(word, "FROM")) {
            word = strtok(NULL, " ");
            char tableName[20];
            strcpy(tableName, word);
            // not impelemented
        }
    }
    else if (!strcmp(word, "SELECT")) {
        word = strtok(NULL, " ");
        // not implemented
    }
    return res;
}