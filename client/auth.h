int login(int socketfd, char *username, char *password, int root) {
    char data[100], buffer[BUFSIZ];
    sprintf(data, "%s,%s", username, password);
    if (root)
        send(socketfd, "root", 4, 0);
    else 
        send(socketfd, data, strlen(data), 0);

    read(socketfd, buffer, BUFSIZ);
    if (strcmp(buffer, OK))
        return 0;
    return 1;
}