/* Shared input helper: asks for a whole number and re-asks until it
 * is inside [lo, hi]. If def is inside the range it is shown in
 * [brackets] and pressing Enter on an empty line picks it, so every
 * demo can be driven with custom values or walked through quickly
 * with the defaults.
 */
#include <stdio.h>
#include <stdlib.h>
#include "demos.h"

int ask_int(const char *prompt, int def, int lo, int hi)
{
    char line[64];
    int v;

    while (1) {
        if (def >= lo && def <= hi)
            printf("%s [%d]: ", prompt, def);
        else
            printf("%s: ", prompt);
        fflush(stdout);

        if (fgets(line, sizeof line, stdin) == NULL) {
            printf("\n");
            exit(0); /* end of input (Ctrl+D) - quit cleanly */
        }
        if (line[0] == '\n' && def >= lo && def <= hi)
            return def;
        if (sscanf(line, "%d", &v) == 1 && v >= lo && v <= hi)
            return v;
        printf("Please enter a number between %d and %d.\n", lo, hi);
    }
}
