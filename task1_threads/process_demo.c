/* Process creation demo: parent forks N children and waits for them. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "demos.h"

#define MAX_CHILDREN 10

void process_demo(void)
{
    pid_t pids[MAX_CHILDREN];
    int n, i;

    printf("\n--- Process creation demo ---\n");
    n = ask_int("How many child processes (1-10)", 3, 1, MAX_CHILDREN);
    printf("Parent process PID: %d\n", getpid());

    for (i = 0; i < n; i++) {
        pid_t pid;

        /* flush before fork, otherwise the child inherits a copy of
         * the parent's stdout buffer and prints it all again */
        fflush(stdout);
        pid = fork();

        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) {
            /* this code runs in the child process */
            printf("Child %d started, PID=%d, parent PID=%d\n",
                   i + 1, getpid(), getppid());
            sleep(1); /* pretend to do some work */
            printf("Child %d (PID=%d) finished\n", i + 1, getpid());
            exit(i + 1); /* exit code = child number, parent reads it back */
        }
        pids[i] = pid;
    }

    /* parent waits for all children so no zombies are left */
    for (i = 0; i < n; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        if (WIFEXITED(status))
            printf("Parent: child PID=%d exited with code %d\n",
                   pids[i], WEXITSTATUS(status));
    }
    printf("All children finished, parent continues.\n");
}
