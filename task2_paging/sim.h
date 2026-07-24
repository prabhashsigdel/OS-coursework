#ifndef SIM_H
#define SIM_H

#define MAX_REFS 200
#define MAX_FRAMES 20

/* result of one simulation run */
struct results {
    int faults;
    int hits;
};

struct results run_fifo(int pages[], int n, int nframes, int verbose);
struct results run_lru(int pages[], int n, int nframes, int verbose);
void print_frames(int frames[], int nframes);

#endif
