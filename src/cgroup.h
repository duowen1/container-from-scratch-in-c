#ifndef _CGROUP_H
#define _CGROUP_H

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define FREEZE 0
#define THAWED 1

void cgroup(pid_t, char *);
int freeze(char *,int);



#define MEMORY_LIMIT 512*1024*1024 //512M

#endif