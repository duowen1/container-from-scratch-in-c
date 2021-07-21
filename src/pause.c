#include "pause.h"

int container_pause(char *argv[]){//todo
    checkroot();
    freeze(argv[2],FREEZE);
    return 0;
}

int container_unpause(char *argv[]){//todo
    checkroot();
    freeze(argv[2],THAWED);
    return 0;
}