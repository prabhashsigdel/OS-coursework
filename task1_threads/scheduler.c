/* Round robin scheduler simulation.
 * Simulated processes (not real ones) with burst times get CPU slices
 * of a fixed quantum, cycling until all are done. Prints the execution
 * order plus turnaround and waiting time for each process.
 */
#include <stdio.h>
#include "demos.h"

struct proc {
    int pid;
    int burst;     /* total CPU time needed */
    int remaining; /* time still left */
    int finish;    /* clock time when it completed */
};

#define MAX_PROCS 10

void scheduler_demo(void)
{
    struct proc procs[MAX_PROCS];
    int def_burst[MAX_PROCS] = {10, 4, 7, 3, 5, 5, 5, 5, 5, 5};
    int n, quantum;
    int time = 0;
    int done = 0;
    int i;
    double avg_tat = 0, avg_wait = 0;
    char msg[32];

    printf("\n--- Round robin scheduler simulation ---\n");
    printf("(press Enter at any prompt to keep the default example)\n");
    n = ask_int("Number of processes (1-10)", 4, 1, MAX_PROCS);
    for (i = 0; i < n; i++) {
        snprintf(msg, sizeof msg, "Burst time for P%d", i + 1);
        procs[i].pid = i + 1;
        procs[i].burst = ask_int(msg, def_burst[i], 1, 100);
        procs[i].finish = 0;
    }
    quantum = ask_int("Time quantum", 3, 1, 20);

    printf("\nTime quantum = %d\n\n", quantum);
    printf("PID  Burst\n");
    for (i = 0; i < n; i++) {
        procs[i].remaining = procs[i].burst;
        printf("%3d  %5d\n", procs[i].pid, procs[i].burst);
    }
    printf("\nExecution order:\n");

    /* keep cycling through the ready processes until all finish */
    while (done < n) {
        for (i = 0; i < n; i++) {
            int slice;

            if (procs[i].remaining == 0)
                continue;

            slice = procs[i].remaining < quantum ? procs[i].remaining : quantum;
            printf("t=%2d..%2d  P%d runs %d unit(s)",
                   time, time + slice, procs[i].pid, slice);
            time += slice;
            procs[i].remaining -= slice;

            if (procs[i].remaining == 0) {
                procs[i].finish = time;
                done++;
                printf("  -> finished");
            }
            printf("\n");
        }
    }

    /* all processes arrive at t=0 here, so:
     * turnaround = finish time, waiting = turnaround - burst */
    printf("\nPID  Burst  Turnaround  Waiting\n");
    for (i = 0; i < n; i++) {
        int tat = procs[i].finish;
        int wait = tat - procs[i].burst;
        avg_tat += tat;
        avg_wait += wait;
        printf("%3d  %5d  %10d  %7d\n",
               procs[i].pid, procs[i].burst, tat, wait);
    }
    printf("\nAverage turnaround: %.2f\n", avg_tat / n);
    printf("Average waiting:    %.2f\n", avg_wait / n);
}
