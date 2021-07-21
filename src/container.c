#include "container.h"

int main(int argc, char *argv[]){
    if(!strcmp(argv[1],"exec")){//如果是exec命令
        container_exec(argv);
    }

    if(!strcmp(argv[1],"run")){//如果是run命令
        container_run(argv);
    }

    if(!strcmp(argv[1],"cp")){//如果是cp命令
        container_cp(argv);
    }

    if(!strcmp(argv[1],"ps")){//如果是ps命令
        container_ps(argv);
    }

    if(!strcmp(argv[1],"help")){//如果是help命令
        container_help();
    }

    return 0;
}

int container_cp(char * argv[]){//目前仅支持从文件复制到容器内，在没有实现unionfs的情况下感觉没啥意义 todo
    //container cp <file path> <file path>   
    return 0;
}

int container_help(){//todo
    printf("Usage: container <command> [ops]\n");
    return 0;
}

int container_ps(){//todo
    return 0;
}