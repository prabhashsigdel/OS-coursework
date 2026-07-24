#ifndef FSEC_H
#define FSEC_H

#define NAME_LEN 32
#define LINE_LEN 256
#define STORE_DIR "store"
#define USER_FILE "users.txt"
#define META_FILE "filemeta.txt"

struct user {
    char name[NAME_LEN];
    char group[NAME_LEN];
    int logged_in;
};

/* auth.c */
int register_user(void);
int login(struct user *u);
void read_line(const char *msg, char *buf, int size);
void read_password(char *buf, int size);

/* fileops.c */
void list_files(struct user *u);
void create_file(struct user *u);
void read_file(struct user *u);
void write_file(struct user *u);
void delete_file(struct user *u);
void set_perms(struct user *u);
void execute_file(struct user *u);
int check_perm(struct user *u, const char *fname, char what); /* 'r' 'w' 'x' */
void path_of(const char *fname, char *out, int size);

/* crypto.c */
void crypt_file(struct user *u, const char *action);

/* audit.c */
void audit(const char *user, const char *action, const char *target,
           const char *result);
void view_audit(struct user *u);

#endif
