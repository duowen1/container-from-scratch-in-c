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
#include <linux/limits.h>
#include <sched.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define STACK_SIZE (1024*1024)
#define CLONE_NEWTIME 0x80
#define CONTAINER_NAME_LEN 12

static char child_stack[STACK_SIZE];//stack of child process

int container_run(char **);
int container_exec(char **);
int container_cp(char **);
int container_ps();
int container_help();

int clean_up(char *);

int checkroot();
char generate_random_char();
char * generate_random_string();

char* init_unionfs();
int setup_hostname(char *);
int setup_network();
int setup_rootfs();
int setup_proc();

const char *NSS[] = {
    "mnt",
    "uts",
    "ipc",
    "net",
    "cgroup",
    "pid",
};