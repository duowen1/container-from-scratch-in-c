#ifndef _EXEC_H
#define _EXEC_H

#include "utils.h"
#include <sys/wait.h>
#include <limits.h>
#include <fcntl.h>

extern int container_exec(char **);

#endif