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
            char *username;
            username = strtok(data, " ");
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
    fpUser = fopen("databases/credentials/users.csv", "a+");

    fprintf(fpUser, "%s,%s\n", username, password);
    fclose(fpUser);

    printf("save user success\n");
}

void giveAccess(char dbname[], char username[]) {
    FILE *fpAccess;
    fpAccess = fopen("databases/credentials/users.csv", "a+");

    fprintf(fpAccess, "%s,%s\n", dbname, username);
    fclose(fpAccess);

    printf("give access user success");
}

int authInterface(char* buffer) {
    char query[100];
    char *word;
    strcpy(query, buffer);
    word = strtok(query, " ");

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
                    password[strcspn(password, "\n")] = 0;
                    saveUser(username, password);
                    res = 1;
                } 
            }
        }
    } 
    else if (!strcmp(word, "USE")) {
        word = strtok(NULL, " ");
        strcpy(activeDB, word);
        printf("Change active DB to %s\n", activeDB);
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
                giveAccess(dbname, username);
                res = 1;
            }
        }
    }
    return res;
}