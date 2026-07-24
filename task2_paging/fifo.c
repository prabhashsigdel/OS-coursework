#include <stdio.h>
#include "sim.h"

/* FIFO: replace the page that has been in memory the longest */
struct results run_fifo(int pages[], int n, int nframes, int verbose)
{
    int frames[MAX_FRAMES];
    int next = 0;               /* points at the oldest frame */
    struct results r = {0, 0};
    int i, j, found;

    for (i = 0; i < nframes; i++)
        frames[i] = -1;         /* -1 means the frame is empty */

    for (i = 0; i < n; i++) {
        found = 0;
        for (j = 0; j < nframes; j++) {
            if (frames[j] == pages[i]) {
                found = 1;
                break;
            }
        }
        if (found) {
            r.hits++;
        } else {
            frames[next] = pages[i];
            next = (next + 1) % nframes;
            r.faults++;
        }
        if (verbose) {
            printf("page %2d -> %s  ", pages[i], found ? "HIT  " : "FAULT");
            print_frames(frames, nframes);
        }
    }
    return r;
}
