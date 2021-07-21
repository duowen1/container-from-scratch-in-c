#ifndef _PAUSE_H
#define _PAUSE_H

#include "utils.h"
#include "cgroup.h"

extern int container_pause(char **);
extern int container_unpause(char **);

#endif