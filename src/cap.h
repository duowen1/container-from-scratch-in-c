#ifndef _CAP_H
#define _CAP_H

#include <sys/capability.h>

void list_capability(int flag);
void init_capability();

#define HOST 0
#define SANDBOX 1

#endif