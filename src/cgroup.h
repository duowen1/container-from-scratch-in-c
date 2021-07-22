#ifndef _CGROUP_H
#define _CGROUP_H

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define FREEZE 0
#define THAWED 1

extern void cgroup(pid_t, char *);
extern int freeze(char *,int);

int init_cpuset_cgroup(pid_t, char *);
int init_cpu_cgroup(pid_t, char *);
int init_memory_cgroup(pid_t, char *);
int init_freezer_cgroup(pid_t, char *);
int cgroup_mkdir_name(int, char *);
int cgroup_fd_operation(int, char *, char *, char *);

#define MEMORY_LIMIT 512*1024*1024 //512M

#endif