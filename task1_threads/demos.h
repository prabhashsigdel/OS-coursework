#ifndef DEMOS_H
#define DEMOS_H

/* one function per demo, called from the menu in main.c */
void process_demo(void);
void race_demo(void);
void prodcons_demo(void);
void deadlock_demo(void);
void scheduler_demo(void);

/* input.c - number prompt with range check and optional default */
int ask_int(const char *prompt, int def, int lo, int hi);

#endif
