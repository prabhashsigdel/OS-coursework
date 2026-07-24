/* Task 4 - TCP client */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "common.h"

int main(int argc, char *argv[])
{
    int fd, n;
    struct sockaddr_in addr;
    char buf[BUF_SIZE], line[BUF_SIZE];
    const char *host = "127.0.0.1";   /* default: same machine */

    if (argc >= 2)
        host = argv[1];               /* allow: ./client <server-ip> */

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {
        printf("Bad server address: %s\n", host);
        return 1;
    }

    if (connect(fd, (struct sockaddr *)&addr, sizeof addr) < 0) {
        perror("connect");
        printf("Is the server running?\n");
        return 1;
    }

    /* print the server's greeting */
    n = recv(fd, buf, sizeof buf - 1, 0);
    if (n > 0) {
        buf[n] = '\0';
        printf("%s", buf);
    }

    printf("Type commands (LOGIN, ECHO, TIME, ADD, QUIT).\n");
    while (1) {
        printf("> ");
        if (fgets(line, sizeof line, stdin) == NULL)
            break;
        if (send(fd, line, strlen(line), 0) < 0) {
            perror("send");
            break;
        }
        n = recv(fd, buf, sizeof buf - 1, 0);
        if (n <= 0) {
            printf("Server closed the connection.\n");
            break;
        }
        buf[n] = '\0';
        printf("%s", buf);
        if (strncmp(line, "QUIT", 4) == 0)
            break;
    }

    close(fd);
    return 0;
}
