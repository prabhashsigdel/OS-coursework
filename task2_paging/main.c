/* Task 2 - memory management simulator: paging with FIFO and LRU */
#include <stdio.h>
#include <stdlib.h>
#include "sim.h"

void print_frames(int frames[], int nframes)
{
    int j;

    printf("| ");
    for (j = 0; j < nframes; j++) {
        if (frames[j] == -1)
            printf(" . ");
        else
            printf("%2d ", frames[j]);
    }
    printf("|\n");
}

static int read_int(const char *msg)
{
    int v;

    printf("%s", msg);
    while (scanf("%d", &v) != 1) {
        while (getchar() != '\n')
            ;
        printf("Not a number, try again: ");
    }
    return v;
}

static void show_stats(const char *name, struct results r, int n)
{
    printf("%-4s: %2d faults, %2d hits   (hit ratio %5.1f%%, miss ratio %5.1f%%)\n",
           name, r.faults, r.hits,
           100.0 * r.hits / n, 100.0 * r.faults / n);
}

/* run both algorithms on the same reference string and compare */
static void compare(int pages[], int n)
{
    struct results f, l;
    int nframes;

    nframes = read_int("Number of frames (1-20): ");
    if (nframes < 1 || nframes > MAX_FRAMES) {
        printf("Frames must be between 1 and %d.\n", MAX_FRAMES);
        return;
    }

    printf("\n--- FIFO ---\n");
    f = run_fifo(pages, n, nframes, 1);
    printf("\n--- LRU ---\n");
    l = run_lru(pages, n, nframes, 1);

    printf("\n=== Summary: %d references, %d frames ===\n", n, nframes);
    show_stats("FIFO", f, n);
    show_stats("LRU", l, n);
}

static void run_demo(void)
{
    /* classic reference string from OS textbooks */
    int pages[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};

    printf("Demo reference string: 7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1\n");
    compare(pages, 20);
}

static void run_custom(void)
{
    int pages[MAX_REFS];
    int n, i;

    n = read_int("How many page references (1-200): ");
    if (n < 1 || n > MAX_REFS) {
        printf("Out of range.\n");
        return;
    }
    printf("Enter %d page numbers separated by spaces:\n", n);
    for (i = 0; i < n; i++)
        pages[i] = read_int("");
    compare(pages, n);
}

/* configurable page size: turn virtual addresses into page numbers */
static void run_addresses(void)
{
    int pages[MAX_REFS], addrs[MAX_REFS];
    int n, i, pagesize;

    pagesize = read_int("Page size in bytes (e.g. 1024): ");
    if (pagesize < 1) {
        printf("Bad page size.\n");
        return;
    }
    n = read_int("How many virtual addresses (1-200): ");
    if (n < 1 || n > MAX_REFS) {
        printf("Out of range.\n");
        return;
    }
    printf("Enter %d addresses:\n", n);
    for (i = 0; i < n; i++) {
        addrs[i] = read_int("");
        pages[i] = addrs[i] / pagesize;   /* page number = address / page size */
    }
    printf("\nAddress -> page mapping (page size %d):\n", pagesize);
    for (i = 0; i < n; i++)
        printf("  address %6d -> page %d\n", addrs[i], pages[i]);
    compare(pages, n);
}

/* extra: FIFO can get WORSE with more frames (Belady's anomaly) */
static void run_belady(void)
{
    int refs[] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
    struct results a, b;

    printf("Belady's anomaly demo, reference string: 1 2 3 4 1 2 5 1 2 3 4 5\n\n");
    printf("--- FIFO with 3 frames ---\n");
    a = run_fifo(refs, 12, 3, 1);
    printf("\n--- FIFO with 4 frames ---\n");
    b = run_fifo(refs, 12, 4, 1);
    printf("\n3 frames: %d faults\n4 frames: %d faults\n", a.faults, b.faults);
    printf("More frames but MORE faults - that is Belady's anomaly.\n");
    printf("(LRU never shows this because it is a stack algorithm.)\n");
}

int main(void)
{
    int choice = -1;

    printf("=== Task 2: Paging Simulator (FIFO vs LRU) ===\n");
    while (choice != 0) {
        printf("\n 1) demo reference string\n");
        printf(" 2) enter page numbers manually\n");
        printf(" 3) enter virtual addresses + page size\n");
        printf(" 4) Belady's anomaly demo (FIFO)\n");
        printf(" 0) exit\n");
        choice = read_int("choice> ");
        switch (choice) {
        case 1: run_demo(); break;
        case 2: run_custom(); break;
        case 3: run_addresses(); break;
        case 4: run_belady(); break;
        case 0: printf("Bye.\n"); break;
        default: printf("Unknown option.\n");
        }
    }
    return 0;
}
