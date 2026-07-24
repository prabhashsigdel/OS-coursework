#include <stdio.h>
#include "sim.h"

/* LRU: replace the page that was used least recently */
struct results run_lru(int pages[], int n, int nframes, int verbose)
{
    int frames[MAX_FRAMES];
    int last_used[MAX_FRAMES];  /* when each frame was last touched */
    struct results r = {0, 0};
    int i, j, found, victim;

    for (i = 0; i < nframes; i++) {
        frames[i] = -1;
        last_used[i] = -1;
    }

    for (i = 0; i < n; i++) {
        found = 0;
        for (j = 0; j < nframes; j++) {
            if (frames[j] == pages[i]) {
                last_used[j] = i;   /* refresh its age on a hit */
                found = 1;
                break;
            }
        }
        if (found) {
            r.hits++;
        } else {
            /* take an empty frame if there is one, else the oldest */
            victim = 0;
            for (j = 0; j < nframes; j++) {
                if (frames[j] == -1) {
                    victim = j;
                    break;
                }
                if (last_used[j] < last_used[victim])
                    victim = j;
            }
            frames[victim] = pages[i];
            last_used[victim] = i;
            r.faults++;
        }
        if (verbose) {
            printf("page %2d -> %s  ", pages[i], found ? "HIT  " : "FAULT");
            print_frames(frames, nframes);
        }
    }
    return r;
}
