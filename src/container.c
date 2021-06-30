#define _GNU_SOURCE

#include "cap.h"
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sched.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define STACK_SIZE (1024*1024)
#define CLONE_NEWTIME 0x80

static char child_stack[STACK_SIZE];//stack of child process

void cgroup(pid_t);
void init_seccomp();

//the function which new process executed
static int childfunction(void *arg){
    int res;
    //caps = cap_get_proc();
    list_capability(2);

    char *hostname=((char **)arg)[1];
    sethostname(hostname,strlen(hostname));
    setdomainname(hostname,strlen(hostname));
    printf("[new]childFunc(): PID = %ld\n",(long)getpid());
    printf("[new]childFunc(): PPID = %ld\n", (long)getppid());

    sleep(1);//wait the parent namespace to set network
    //setup container network
    system("ip link set lo up");//turn on loop back address
    system("ip link set veth1 up");//turn on the network advice
    system("ip addr add 192.168.31.10/24 dev veth1");//set the ip address on device
    //we use route command, which is not existed in the new filesystem, so we must execute this commond before chroot
    system("route add default gw 192.168.31.1 veth1");//add the net gate address to iptables

    //change root directory to rootfs
    char *rootfs=((char **)arg)[2];
    res=chroot(rootfs);
    if(res<0){
        perror("Chroot fail");
    }
    chdir("/");//change pwd to rootfs,without this the container might escape
    printf("[new]chroot success\n");

    //mount the proc and then the ps commond can show the right result in new PID namespace
    char *mount_point = "proc";
    if(mount_point != NULL){
        mkdir(mount_point,0555);
        if(mount("proc",mount_point,"proc",0,NULL)==-1){
            perror("Mount fail");
            exit(1);
        }else{
            printf("[new]Mounting procfs as %s\n",mount_point);
        }
    }

    init_capability();
    init_seccomp();

    char * args=NULL;
    execv("/bin/bash", &args);
    return 0;
}

int main(int argc, char *argv[]){
    list_capability(1);
    pid_t child_pid;
    struct utsname uts;
    if(unshare(CLONE_NEWTIME)){
        perror("make new time namespace fail.\n");
        return 0;
    }
    int flag=CLONE_NEWUTS | CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWNET ;
    child_pid=clone(childfunction,child_stack+STACK_SIZE,flag | SIGCHLD,(void*)argv);
    if(child_pid==-1){
        //output error information and exit
        perror("Create process fail");
        exit(1);
    }else{
        cgroup(child_pid);//set cgroup rules

        system("ip link add veth0 type veth peer name veth1");
        char cmd[100];
        sprintf(cmd,"ip link set veth1 netns %d",child_pid);
        system(cmd);
        system("ip link set veth0 up");
        system("ip addr add 192.168.31.1/24 dev veth0");
        if(uname(&uts)==-1){//print the uts from host
            //output error information and exit
            perror("uname fail: ");
            exit(1);

        }else{
            printf("[old]PID returned by clone(): %ld\n",(long)child_pid);
            printf("[old]uts.nodename in parent : %s\n",uts.nodename);
        }
        sleep(2);
        if(waitpid(child_pid,NULL,0)==-1){
            //output error information and exit
            perror("child terminate fail:");
            exit(1);

        }else{
            printf("[old]child has terminated\n");
        }
    }
    return 0;
}