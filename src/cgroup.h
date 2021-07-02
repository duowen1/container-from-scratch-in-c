#ifndef _CGROUP_H
#define _CGROUP_H

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern void cgroup(pid_t);
int init_cpuset_cgroup(pid_t);
int init_cpu_cgroup(pid_t);

#endif