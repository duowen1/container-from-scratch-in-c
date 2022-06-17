#ifndef _RUN_H
#define _RUN_H

#define _GNU_SOURCE

#include "cap.h"
#include "cgroup.h"
#include "comp.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sched.h>
#include <fcntl.h>
#include <errno.h>

#define STACK_SIZE (1024*1024)
#define CONTAINER_NAME_LEN 12

static char child_stack[STACK_SIZE];//stack of child process

extern int container_run(char **);


#endif