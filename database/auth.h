int login(int socketfd, char *data) {
    if (!strcmp(data, "root"))
        return 1;

    FILE *fp = fopen("account.csv", "r");
    char line[100];
    int is_exist = 0;

    while (fscanf(fp, "%s", line) != EOF) {
        if (strcmp(data, line) == 0) {
            strcpy(activeUser, data);
            is_exist = 1;
            break;
        }
    }
    if (is_exist) 
        return 1;
    return 0;
}


void saveUser(char username[], char password[]) {
    FILE *fpUser;
    fpUser = fopen("account.csv", "a+");

    fprintf(fpUser, "%s,%s\n", username, password);
    fclose(fpUser);

    printf("save user success\n");
}

void giveAccess(char username[], char dbname[]) {
    // Not implemented
}

int authInterface(char* buffer, char* word) {
    int res = 0;
    if (!strcmp(word, "CREATE")) {
        word = strtok(NULL, " ");
        if (!strcmp(word, "USER")) {
            word = strtok(NULL, " ");
            char username[20], password[20];
            strcpy(username, word);
            word = strtok(NULL, " ");
            if (!strcmp(word, "IDENTIFIED")) {
                word = strtok(NULL, " ");
                if (!strcmp(word, "BY")) {
                    word = strtok(NULL, " ");
                    strcpy(password, word);
                    saveUser(username, password);
                    res = 1;
                } 
            }
        }
    } 
    else if (!strcmp(word, "USE")) {
        word = strtok(NULL, " ");
        strcpy(activeDB, word);
    }
    else if (!strcmp(word, "GRANT")) {
        word = strtok(NULL, " ");
        if (!strcmp(word, "PERMISSION")) {
            word = strtok(NULL, " ");
            char dbname[20], username[20];
            strcpy(dbname, word);
            if (!strcmp(word, "INTO")) {
                word = strtok(NULL, " ");
                strcpy(username, word);
                giveAccess(username, dbname);
                res = 1;
            }
        }
    }
    return res;
}