#ifndef _CAP_H
#define _CAP_H

#include <sys/capability.h>

extern void list_capability(int flag);
extern void init_capability();

#define HOST 0
#define SANDBOX 1

#endif