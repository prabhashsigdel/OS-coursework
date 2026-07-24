#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fsec.h"

/* XOR stream cipher: each byte is XORed with a key byte that also
   depends on the position, so the pattern does not simply repeat.
   XOR is symmetric, so the exact same operation decrypts too.
   (fine for coursework - a real system would use AES.) */
static void xor_buffer(unsigned char *buf, long n, const char *key)
{
    long i;
    int klen = strlen(key);

    for (i = 0; i < n; i++)
        buf[i] ^= (unsigned char)(key[i % klen] + i / klen);
}

void crypt_file(struct user *u, const char *action)
{
    char fname[NAME_LEN], path[LINE_LEN], key[64];
    unsigned char *buf;
    long n;
    FILE *f;
    int r, w;

    read_line("File name: ", fname, sizeof fname);
    r = check_perm(u, fname, 'r');
    w = check_perm(u, fname, 'w');
    if (r < 0) {
        printf("No such file.\n");
        return;
    }
    if (r == 0 || w == 0) {
        printf("Permission denied (need read + write).\n");
        audit(u->name, action, fname, "DENIED");
        return;
    }
    printf("Key: ");
    read_password(key, sizeof key);
    if (key[0] == '\0') {
        printf("Empty key.\n");
        return;
    }

    path_of(fname, path, sizeof path);
    f = fopen(path, "rb");
    if (!f) {
        perror("open");
        return;
    }
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    rewind(f);
    buf = malloc(n > 0 ? n : 1);
    if (buf == NULL) {
        printf("Out of memory.\n");
        fclose(f);
        return;
    }
    if (n > 0 && fread(buf, 1, n, f) != (size_t)n) {
        printf("Read error.\n");
        free(buf);
        fclose(f);
        return;
    }
    fclose(f);

    xor_buffer(buf, n, key);

    f = fopen(path, "wb");
    if (!f) {
        perror("open");
        free(buf);
        return;
    }
    fwrite(buf, 1, n, f);
    fclose(f);
    free(buf);

    audit(u->name, action, fname, "ok");
    printf("%s done (%ld bytes). The same key reverses it.\n", action, n);
}
