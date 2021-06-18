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

void saveUser(char username[], char password[]) {
    FILE *fpUser;
    fpUser = fopen("databases/credentials/users.csv", "a+");

    fprintf(fpUser, "%s,%s\n", username, password);
    fclose(fpUser);
    
    FILE *fpTemp = fopen(tempPath, "w");
    fprintf(fpTemp, "User created");
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
                    saveUser(username, password);
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