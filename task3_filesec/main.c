/* Task 3 - secure file manager: auth, permissions, encryption, audit log */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "fsec.h"

static int menu_choice(void)
{
    char line[16];

    printf("choice> ");
    if (fgets(line, sizeof line, stdin) == NULL)
        return 0;               /* EOF acts like exit/logout */
    return atoi(line);
}

static void session(struct user *u)
{
    int c;

    while (u->logged_in) {
        printf("\n-- logged in as %s (group %s) --\n", u->name, u->group);
        printf("  1) list files       2) create file\n");
        printf("  3) read file        4) write to file\n");
        printf("  5) delete file      6) change permissions\n");
        printf("  7) encrypt file     8) decrypt file\n");
        printf("  9) execute file    10) view audit log\n");
        printf("  0) logout\n");
        c = menu_choice();
        switch (c) {
        case 1: list_files(u); break;
        case 2: create_file(u); break;
        case 3: read_file(u); break;
        case 4: write_file(u); break;
        case 5: delete_file(u); break;
        case 6: set_perms(u); break;
        case 7: crypt_file(u, "ENCRYPT"); break;
        case 8: crypt_file(u, "DECRYPT"); break;
        case 9: execute_file(u); break;
        case 10: view_audit(u); break;
        case 0:
            audit(u->name, "LOGOUT", "-", "ok");
            u->logged_in = 0;
            break;
        default:
            printf("Unknown option.\n");
        }
    }
}

int main(void)
{
    struct user u;
    int c, running = 1;

    srand((unsigned)time(NULL));
    mkdir(STORE_DIR, 0700);     /* no problem if it already exists */

    printf("=== Task 3: Secure File Manager ===\n");
    while (running) {
        printf("\n 1) register   2) login   0) exit\n");
        c = menu_choice();
        switch (c) {
        case 1:
            register_user();
            break;
        case 2:
            memset(&u, 0, sizeof u);
            if (login(&u))
                session(&u);
            break;
        case 0:
            running = 0;
            break;
        default:
            printf("Unknown option.\n");
        }
    }
    printf("Bye.\n");
    return 0;
}
