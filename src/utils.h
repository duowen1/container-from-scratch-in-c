#ifndef _UTILS_H
#define _UTILS_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>

void checkroot();
char *generate_random_string(int len);
int isstartwith(char *, char *);

#endif