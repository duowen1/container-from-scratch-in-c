#define _GNU_SOURCE

#include "cap.h"
#include "comp.h"
#include "cgroup.h"
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sched.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define STACK_SIZE (1024*1024)
#define CLONE_NEWTIME 0x80

static char child_stack[STACK_SIZE];//stack of child process

int setup_hostname(char *);
int setup_network();
int setup_rootfs();
int setup_proc();