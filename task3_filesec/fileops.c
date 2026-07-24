#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include "fsec.h"

void path_of(const char *fname, char *out, int size)
{
    snprintf(out, size, "%s/%s", STORE_DIR, fname);
}

/* only allow simple names - blocks path traversal like ../../etc/passwd */
static int valid_name(const char *s)
{
    int i;

    if (s[0] == '\0' || strlen(s) > 30)
        return 0;
    for (i = 0; s[i]; i++) {
        if (!isalnum((unsigned char)s[i]) && s[i] != '.' && s[i] != '_'
            && s[i] != '-')
            return 0;
    }
    if (strcmp(s, ".") == 0 || strcmp(s, "..") == 0)
        return 0;
    return 1;
}

/* filemeta.txt format: name:owner:group:mode(octal) */
static int find_meta(const char *fname, char *owner, char *group,
                     unsigned *mode)
{
    FILE *f = fopen(META_FILE, "r");
    char line[LINE_LEN], name[NAME_LEN], own[NAME_LEN], grp[NAME_LEN];
    unsigned m;
    int found = 0;

    if (!f)
        return 0;
    while (fgets(line, sizeof line, f)) {
        if (sscanf(line, "%31[^:]:%31[^:]:%31[^:]:%o", name, own, grp, &m) == 4
            && strcmp(name, fname) == 0) {
            if (owner)
                strcpy(owner, own);
            if (group)
                strcpy(group, grp);
            if (mode)
                *mode = m;
            found = 1;
            break;
        }
    }
    fclose(f);
    return found;
}

/* replace a file's meta line (newline != NULL) or remove it (newline == NULL) */
static int update_meta(const char *fname, const char *newline)
{
    FILE *in = fopen(META_FILE, "r");
    FILE *out = fopen("filemeta.tmp", "w");
    char line[LINE_LEN], name[NAME_LEN];

    if (!in || !out) {
        if (in)
            fclose(in);
        if (out)
            fclose(out);
        return 0;
    }
    while (fgets(line, sizeof line, in)) {
        if (sscanf(line, "%31[^:]", name) == 1 && strcmp(name, fname) == 0) {
            if (newline)
                fputs(newline, out);
            /* else skip the line = delete */
        } else {
            fputs(line, out);
        }
    }
    fclose(in);
    fclose(out);
    remove(META_FILE);
    rename("filemeta.tmp", META_FILE);
    return 1;
}

/* decide which permission bits apply: owner, group or others */
int check_perm(struct user *u, const char *fname, char what)
{
    char owner[NAME_LEN], group[NAME_LEN];
    unsigned mode;
    int shift, bit;

    if (!find_meta(fname, owner, group, &mode))
        return -1;              /* file does not exist */

    if (strcmp(u->name, owner) == 0)
        shift = 6;              /* owner bits  rwx------ */
    else if (strcmp(u->group, group) == 0)
        shift = 3;              /* group bits  ---rwx--- */
    else
        shift = 0;              /* other bits  ------rwx */

    if (what == 'r')
        bit = 4;
    else if (what == 'w')
        bit = 2;
    else
        bit = 1;

    return ((mode >> shift) & bit) ? 1 : 0;
}

static void mode_string(unsigned mode, char out[10])
{
    const char *rwx = "rwxrwxrwx";
    int i;

    for (i = 0; i < 9; i++)
        out[i] = (mode & (1u << (8 - i))) ? rwx[i] : '-';
    out[9] = '\0';
}

void list_files(struct user *u)
{
    FILE *f = fopen(META_FILE, "r");
    char line[LINE_LEN], name[NAME_LEN], owner[NAME_LEN], group[NAME_LEN];
    char ms[10];
    unsigned mode;
    int count = 0;

    (void)u;
    if (!f) {
        printf("No files yet.\n");
        return;
    }
    printf("%-20s %-10s %-10s %s\n", "file", "owner", "group", "perms");
    while (fgets(line, sizeof line, f)) {
        if (sscanf(line, "%31[^:]:%31[^:]:%31[^:]:%o", name, owner, group,
                   &mode) == 4) {
            mode_string(mode, ms);
            printf("%-20s %-10s %-10s %s (%o)\n", name, owner, group, ms,
                   mode);
            count++;
        }
    }
    fclose(f);
    if (count == 0)
        printf("(none)\n");
}

void create_file(struct user *u)
{
    char fname[NAME_LEN], path[LINE_LEN], modestr[16];
    unsigned mode;
    FILE *f, *meta;

    read_line("File name: ", fname, sizeof fname);
    if (!valid_name(fname)) {
        printf("Bad file name (letters, digits, . _ - only).\n");
        return;
    }
    if (find_meta(fname, NULL, NULL, NULL)) {
        printf("File already exists.\n");
        return;
    }
    read_line("Permissions in octal (default 640 = rw-r-----): ", modestr,
              sizeof modestr);
    if (modestr[0] == '\0')
        strcpy(modestr, "640");
    mode = (unsigned)strtol(modestr, NULL, 8);
    if (mode > 0777) {
        printf("Bad mode.\n");
        return;
    }

    path_of(fname, path, sizeof path);
    f = fopen(path, "w");
    if (!f) {
        perror("create");
        return;
    }
    fclose(f);
    chmod(path, mode);          /* mirror the mode onto the real file */

    meta = fopen(META_FILE, "a");
    if (!meta) {
        perror("filemeta.txt");
        return;
    }
    fprintf(meta, "%s:%s:%s:%o\n", fname, u->name, u->group, mode);
    fclose(meta);

    audit(u->name, "CREATE", fname, "ok");
    printf("Created '%s' with mode %o.\n", fname, mode);
}

void read_file(struct user *u)
{
    char fname[NAME_LEN], path[LINE_LEN], line[LINE_LEN];
    FILE *f;
    int ok;

    read_line("File to read: ", fname, sizeof fname);
    ok = check_perm(u, fname, 'r');
    if (ok < 0) {
        printf("No such file.\n");
        return;
    }
    if (ok == 0) {
        printf("Permission denied.\n");
        audit(u->name, "READ", fname, "DENIED");
        return;
    }
    path_of(fname, path, sizeof path);
    f = fopen(path, "r");
    if (!f) {
        perror("read");
        return;
    }
    printf("--- %s ---\n", fname);
    while (fgets(line, sizeof line, f))
        fputs(line, stdout);
    printf("\n--- end ---\n");
    fclose(f);
    audit(u->name, "READ", fname, "ok");
}

void write_file(struct user *u)
{
    char fname[NAME_LEN], path[LINE_LEN], text[LINE_LEN], how[8];
    FILE *f;
    int ok;

    read_line("File to write: ", fname, sizeof fname);
    ok = check_perm(u, fname, 'w');
    if (ok < 0) {
        printf("No such file.\n");
        return;
    }
    if (ok == 0) {
        printf("Permission denied.\n");
        audit(u->name, "WRITE", fname, "DENIED");
        return;
    }
    read_line("a = append, o = overwrite: ", how, sizeof how);
    read_line("Text: ", text, sizeof text);
    path_of(fname, path, sizeof path);
    f = fopen(path, how[0] == 'o' ? "w" : "a");
    if (!f) {
        perror("write");
        return;
    }
    fprintf(f, "%s\n", text);
    fclose(f);
    audit(u->name, "WRITE", fname, "ok");
    printf("Saved.\n");
}

void delete_file(struct user *u)
{
    char fname[NAME_LEN], owner[NAME_LEN], group[NAME_LEN], path[LINE_LEN];
    unsigned mode;

    read_line("File to delete: ", fname, sizeof fname);
    if (!find_meta(fname, owner, group, &mode)) {
        printf("No such file.\n");
        return;
    }
    if (strcmp(u->name, owner) != 0) {
        printf("Only the owner can delete a file.\n");
        audit(u->name, "DELETE", fname, "DENIED");
        return;
    }
    path_of(fname, path, sizeof path);
    remove(path);
    update_meta(fname, NULL);
    audit(u->name, "DELETE", fname, "ok");
    printf("Deleted '%s'.\n", fname);
}

void set_perms(struct user *u)
{
    char fname[NAME_LEN], owner[NAME_LEN], group[NAME_LEN];
    char modestr[16], nl[LINE_LEN], path[LINE_LEN];
    unsigned mode;

    read_line("File: ", fname, sizeof fname);
    if (!find_meta(fname, owner, group, &mode)) {
        printf("No such file.\n");
        return;
    }
    if (strcmp(u->name, owner) != 0) {
        printf("Only the owner can change permissions.\n");
        audit(u->name, "CHMOD", fname, "DENIED");
        return;
    }
    read_line("New mode in octal (e.g. 640): ", modestr, sizeof modestr);
    mode = (unsigned)strtol(modestr, NULL, 8);
    if (mode > 0777) {
        printf("Bad mode.\n");
        return;
    }
    snprintf(nl, sizeof nl, "%s:%s:%s:%o\n", fname, owner, group, mode);
    update_meta(fname, nl);
    path_of(fname, path, sizeof path);
    chmod(path, mode);
    audit(u->name, "CHMOD", fname, "ok");
    printf("Mode of '%s' is now %o.\n", fname, mode);
}

void execute_file(struct user *u)
{
    char fname[NAME_LEN], cmd[LINE_LEN + 8];
    int ok;

    read_line("File to execute: ", fname, sizeof fname);
    ok = check_perm(u, fname, 'x');
    if (ok < 0) {
        printf("No such file.\n");
        return;
    }
    if (ok == 0) {
        printf("Permission denied (no execute bit for you).\n");
        audit(u->name, "EXEC", fname, "DENIED");
        return;
    }
    /* fname is already restricted by valid_name at creation time */
    snprintf(cmd, sizeof cmd, "./%s/%s", STORE_DIR, fname);
    printf("Running %s ...\n", cmd);
    ok = system(cmd);
    printf("(exit status %d)\n", ok);
    audit(u->name, "EXEC", fname, "ok");
}
