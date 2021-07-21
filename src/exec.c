#include "exec.h"

const char *NSS[] = {
    "mnt",
    "uts",
    "ipc",
    "net",
    "cgroup",
    "pid",
};

int container_exec(char *argv[]){
    checkroot();
    pid_t target_pid = atoi(argv[2]);
    char fname[PATH_MAX];
    for (int i=0; i < 6; i++){
        snprintf(fname, (int)sizeof(fname),"/proc/%d/ns/%s",target_pid,NSS[i]);
        int ns_fd = open(fname,O_RDONLY);
        setns(ns_fd,0);
        close(ns_fd);
        if(strcmp(NSS[i],"pid")==0){//如果是pid namespace
            if(fork()){//创建子进程，只有创建子进程才能进入
                sleep(1000);//主进程退出
            }else{
                char * args=NULL;
                execv("/bin/bash", &args);
            }
        }
    }
}