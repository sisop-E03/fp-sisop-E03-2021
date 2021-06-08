int login(int socketfd, char *username, char *password) {
    char data[100], buffer[BUFSIZ];
    sprintf(data, "%s,%s", username, password);
    send(socketfd, data, strlen(data), 0);
    read(socketfd, buffer, BUFSIZ);
    if (strcmp(buffer, OK))
        return 0;
    return 1;
}