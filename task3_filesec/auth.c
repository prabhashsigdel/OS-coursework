#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>
#include "fsec.h"

/* salted hash, repeated many rounds to slow down brute forcing.
   a real system would use a library hash like SHA-256 or bcrypt. */
static unsigned long hash_pass(const char *pass, unsigned long salt)
{
    unsigned long h = 5381 ^ salt;
    const char *p;
    int round;

    for (round = 0; round < 5000; round++)
        for (p = pass; *p; p++)
            h = h * 33 + (unsigned char)*p;
    return h;
}

void read_line(const char *msg, char *buf, int size)
{
    printf("%s", msg);
    if (fgets(buf, size, stdin) == NULL)
        buf[0] = '\0';
    buf[strcspn(buf, "\n")] = '\0';
}

/* read a password without echoing it to the screen */
void read_password(char *buf, int size)
{
    struct termios old, new;
    int hidden = 0;

    if (isatty(0) && tcgetattr(0, &old) == 0) {
        new = old;
        new.c_lflag &= ~ECHO;
        tcsetattr(0, TCSANOW, &new);
        hidden = 1;
    }
    if (fgets(buf, size, stdin) == NULL)
        buf[0] = '\0';
    buf[strcspn(buf, "\n")] = '\0';
    if (hidden) {
        tcsetattr(0, TCSANOW, &old);
        printf("\n");
    }
}

/* users.txt format: name:group:salt:hash */
static int find_user(const char *name, unsigned long *salt,
                     unsigned long *hash, char *group)
{
    FILE *f = fopen(USER_FILE, "r");
    char line[LINE_LEN], uname[NAME_LEN], ugroup[NAME_LEN];
    unsigned long s, h;
    int found = 0;

    if (!f)
        return 0;
    while (fgets(line, sizeof line, f)) {
        if (sscanf(line, "%31[^:]:%31[^:]:%lu:%lu", uname, ugroup, &s, &h) == 4
            && strcmp(uname, name) == 0) {
            if (salt)
                *salt = s;
            if (hash)
                *hash = h;
            if (group)
                strcpy(group, ugroup);
            found = 1;
            break;
        }
    }
    fclose(f);
    return found;
}

int register_user(void)
{
    char name[NAME_LEN], group[NAME_LEN], pass[LINE_LEN], pass2[LINE_LEN];
    unsigned long salt;
    FILE *f;

    read_line("New username: ", name, sizeof name);
    if (name[0] == '\0' || strchr(name, ':') != NULL) {
        printf("Invalid username.\n");
        return 0;
    }
    if (find_user(name, NULL, NULL, NULL)) {
        printf("That user already exists.\n");
        return 0;
    }
    read_line("Group (e.g. students, admin): ", group, sizeof group);
    if (group[0] == '\0' || strchr(group, ':') != NULL) {
        printf("Invalid group.\n");
        return 0;
    }
    printf("Password (min 4 chars): ");
    read_password(pass, sizeof pass);
    printf("Repeat password: ");
    read_password(pass2, sizeof pass2);
    if (strlen(pass) < 4 || strcmp(pass, pass2) != 0) {
        printf("Passwords too short or do not match.\n");
        return 0;
    }

    salt = ((unsigned long)rand() << 16) ^ (unsigned long)time(NULL);
    f = fopen(USER_FILE, "a");
    if (!f) {
        perror("users.txt");
        return 0;
    }
    fprintf(f, "%s:%s:%lu:%lu\n", name, group, salt, hash_pass(pass, salt));
    fclose(f);
    chmod(USER_FILE, 0600);   /* keep the password file private */

    audit(name, "REGISTER", "-", "ok");
    printf("User '%s' registered (group %s).\n", name, group);
    return 1;
}

int login(struct user *u)
{
    char name[NAME_LEN], pass[LINE_LEN], group[NAME_LEN];
    unsigned long salt, stored;

    read_line("Username: ", name, sizeof name);
    printf("Password: ");
    read_password(pass, sizeof pass);

    if (find_user(name, &salt, &stored, group)
        && hash_pass(pass, salt) == stored) {
        strcpy(u->name, name);
        strcpy(u->group, group);
        u->logged_in = 1;
        audit(name, "LOGIN", "-", "ok");
        printf("Welcome, %s (group %s).\n", name, group);
        return 1;
    }
    audit(name[0] ? name : "?", "LOGIN", "-", "DENIED");
    printf("Login failed.\n");
    return 0;
}
