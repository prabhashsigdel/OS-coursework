/* Task 4 - concurrent TCP server (one thread per client) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "common.h"

/* hard-coded accounts for the demo. a real server would look these up
   in a database with hashed passwords. */
struct account {
    const char *user;
    const char *pass;
};
static struct account accounts[] = {
    {"alice", "wonderland"},
    {"bob", "builder"},
};
static int num_accounts = 2;

static int client_count = 0;
static pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;

static int check_login(const char *user, const char *pass)
{
    int i;

    for (i = 0; i < num_accounts; i++) {
        if (strcmp(user, accounts[i].user) == 0
            && strcmp(pass, accounts[i].pass) == 0)
            return 1;
    }
    return 0;
}

/* send a line back to the client */
static void reply(int fd, const char *msg)
{
    send(fd, msg, strlen(msg), 0);
}

static void *handle_client(void *arg)
{
    int fd = *(int *)arg;
    char buf[BUF_SIZE], cmd[64], a1[128], a2[128];
    char out[BUF_SIZE + 64];
    int authed = 0, n, parts;
    int now;

    free(arg);

    pthread_mutex_lock(&count_lock);
    client_count++;
    printf("[server] client connected (now %d active)\n", client_count);
    pthread_mutex_unlock(&count_lock);

    reply(fd, "OK server ready. Please LOGIN <user> <pass>\n");

    while ((n = recv(fd, buf, sizeof buf - 1, 0)) > 0) {
        buf[n] = '\0';
        buf[strcspn(buf, "\r\n")] = '\0';   /* trim newline */
        if (buf[0] == '\0')
            continue;

        cmd[0] = a1[0] = a2[0] = '\0';
        parts = sscanf(buf, "%63s %127s %127s", cmd, a1, a2);

        if (strcmp(cmd, "LOGIN") == 0) {
            if (parts >= 3 && check_login(a1, a2)) {
                authed = 1;
                snprintf(out, sizeof out, "OK welcome %s\n", a1);
                printf("[server] login ok: %s\n", a1);
            } else {
                reply(fd, "ERR bad login\n");
                printf("[server] login FAILED for '%s'\n", a1);
                continue;
            }
            reply(fd, out);
        } else if (strcmp(cmd, "QUIT") == 0) {
            reply(fd, "OK bye\n");
            break;
        } else if (!authed) {
            reply(fd, "ERR please login first\n");
        } else if (strcmp(cmd, "ECHO") == 0) {
            /* echo back everything after the word ECHO */
            snprintf(out, sizeof out, "OK %s\n",
                     buf + 4 + (buf[4] == ' ' ? 1 : 0));
            reply(fd, out);
        } else if (strcmp(cmd, "TIME") == 0) {
            time_t t = time(NULL);
            char ts[64];
            strftime(ts, sizeof ts, "%Y-%m-%d %H:%M:%S", localtime(&t));
            snprintf(out, sizeof out, "OK %s\n", ts);
            reply(fd, out);
        } else if (strcmp(cmd, "ADD") == 0) {
            if (parts >= 3) {
                now = atoi(a1) + atoi(a2);
                snprintf(out, sizeof out, "OK %d\n", now);
                reply(fd, out);
            } else {
                reply(fd, "ERR usage: ADD <a> <b>\n");
            }
        } else {
            reply(fd, "ERR unknown command\n");
        }
    }

    close(fd);
    pthread_mutex_lock(&count_lock);
    client_count--;
    printf("[server] client disconnected (now %d active)\n", client_count);
    pthread_mutex_unlock(&count_lock);
    return NULL;
}

int main(void)
{
    int server_fd, *client_fd;
    struct sockaddr_in addr, caddr;
    socklen_t clen;
    int opt = 1;
    pthread_t tid;

    /* line-buffer stdout so log messages show up immediately,
       even when the output is redirected to a file */
    setvbuf(stdout, NULL, _IOLBF, 0);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);

    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof addr) < 0) {
        perror("bind");
        return 1;
    }
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        return 1;
    }
    printf("[server] listening on port %d\n", PORT);

    for (;;) {
        clen = sizeof caddr;
        client_fd = malloc(sizeof(int));
        if (client_fd == NULL)
            continue;
        *client_fd = accept(server_fd, (struct sockaddr *)&caddr, &clen);
        if (*client_fd < 0) {
            perror("accept");
            free(client_fd);
            continue;
        }
        printf("[server] accepted %s:%d\n", inet_ntoa(caddr.sin_addr),
               ntohs(caddr.sin_port));
        /* one detached thread per client = concurrent connections */
        if (pthread_create(&tid, NULL, handle_client, client_fd) != 0) {
            perror("pthread_create");
            close(*client_fd);
            free(client_fd);
            continue;
        }
        pthread_detach(tid);
    }
    close(server_fd);
    return 0;
}
