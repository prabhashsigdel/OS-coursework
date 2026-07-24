/* ST5004CEM Coursework - Task 1: Process Management and Threading
 * Menu driven program, each option runs one demo.
 */
#include <stdio.h>
#include "demos.h"

int main(void)
{
    int choice;

    while (1) {
        printf("\n===== Task 1: Process Management & Threading =====\n");
        printf("1. Process creation demo (fork)\n");
        printf("2. Race condition demo (without/with mutex)\n");
        printf("3. Producer-consumer demo (semaphores)\n");
        printf("4. Deadlock prevention demo (lock ordering)\n");
        printf("5. Round robin scheduler simulation\n");
        printf("0. Exit\n");

        choice = ask_int("Enter choice", -1, 0, 5);

        switch (choice) {
        case 1: process_demo(); break;
        case 2: race_demo(); break;
        case 3: prodcons_demo(); break;
        case 4: deadlock_demo(); break;
        case 5: scheduler_demo(); break;
        case 0: return 0;
        default: printf("Invalid choice, try again.\n");
        }
    }
    return 0;
}
