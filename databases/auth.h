int login(int socketfd, char *data) {
    if (!strcmp(data, "root"))
        return 1;

    FILE *fp = fopen("account.csv", "r");
    char line[100];
    int is_exist = 0;

    while (fscanf(fp, "%s", line) != EOF) {
        if (strcmp(data, line) == 0) {
            is_exist = 1;
            break;
        }
    }
    if (is_exist) 
        return 1;
    return 0;
}