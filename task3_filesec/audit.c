#include <stdio.h>
#include <string.h>
#include <time.h>
#include "fsec.h"

#define LOG_FILE "audit.log"

/* one line per action: when, who, what, on which file, and the outcome */
void audit(const char *user, const char *action, const char *target,
           const char *result)
{
    FILE *f = fopen(LOG_FILE, "a");
    char stamp[32];
    time_t now = time(NULL);

    if (!f)
        return;
    strftime(stamp, sizeof stamp, "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(f, "%s | %-10s | %-8s | %-20s | %s\n", stamp, user, action,
            target, result);
    fclose(f);
}

void view_audit(struct user *u)
{
    FILE *f;
    char line[LINE_LEN];

    if (strcmp(u->group, "admin") != 0) {
        printf("Only users in the 'admin' group can view the audit log.\n");
        audit(u->name, "VIEWLOG", "-", "DENIED");
        return;
    }
    f = fopen(LOG_FILE, "r");
    if (!f) {
        printf("No audit log yet.\n");
        return;
    }
    printf("--- audit log ---\n");
    while (fgets(line, sizeof line, f))
        fputs(line, stdout);
    fclose(f);
    audit(u->name, "VIEWLOG", "-", "ok");
}
