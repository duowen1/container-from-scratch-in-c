#define _GNU_SOURCE
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <sched.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <seccomp.h>

#define STACK_SIZE (1024*1024)

#define CLONE_NEWTIME 0x80

#define N 23

static char child_stack[STACK_SIZE];//stack of child process

void cgroup(pid_t);
void list_capability(int flag);
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

    //spawn a shell

    //I should utilize capabilities to limit

    cap_value_t cap_list[N];
    cap_list[0]=CAP_SYS_CHROOT;
    cap_list[1]=CAP_SYS_ADMIN;
    cap_list[2]=CAP_SYS_TIME;
    cap_list[3]=CAP_SYS_BOOT;
    cap_list[4]=CAP_SYS_RAWIO;
    cap_list[5]=CAP_SYSLOG;
    cap_list[6]=CAP_DAC_READ_SEARCH;
    cap_list[7]=CAP_LINUX_IMMUTABLE;
    cap_list[8]=CAP_NET_BROADCAST;
    cap_list[9]=CAP_NET_ADMIN;
    cap_list[10]=CAP_IPC_LOCK;
    cap_list[11]=CAP_IPC_OWNER;
    cap_list[12]=CAP_SYS_MODULE;
    cap_list[13]=CAP_SYS_PTRACE;
    cap_list[14]=CAP_SYS_PACCT;
    cap_list[15]=CAP_SYS_NICE;
    cap_list[16]=CAP_SYS_RESOURCE;
    cap_list[17]=CAP_SYS_TTY_CONFIG;
    cap_list[18]=CAP_LEASE;
    cap_list[19]=CAP_AUDIT_CONTROL; 
    cap_list[20]=CAP_MAC_OVERRIDE;
    cap_list[21]=CAP_MAC_ADMIN;
    cap_list[21]=CAP_WAKE_ALARM;
    cap_list[22]=CAP_BLOCK_SUSPEND;
    
    for(int i=0;i<N;i++){
        res=prctl(PR_CAPBSET_DROP,cap_list[i]);
        if(res!=0){
            perror("prctl call fail\n");
            return 0;
        }
    }

    init_seccomp();

    execlp("/bin/bash",NULL);
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

void cgroup(pid_t pid){
    FILE *cgroup_cpu_fd = NULL;
    FILE *cgroup_procs_fd = NULL;
    mkdir("/sys/fs/cgroup/cpu/group1/",0755);

    cgroup_cpu_fd=fopen("/sys/fs/cgroup/cpu/group1/cpu.cfs_quota_us","w");
    if(cgroup_cpu_fd==NULL){
        perror("Open fail fail");
        exit(1);
    }
    fprintf(cgroup_cpu_fd,"%d",100000);
    fclose(cgroup_cpu_fd);

    cgroup_procs_fd=fopen("/sys/fs/cgroup/cpu/group1/cgroup.procs","w");
    if(cgroup_procs_fd==NULL){
        perror("Open file fail");
        exit(1);
    }
    fprintf(cgroup_procs_fd,"%d",pid);
    fclose(cgroup_procs_fd);
    printf("Creat cgroup success\n");
    return;
}

void list_capability(int flag){
    struct __user_cap_header_struct cap_header_data;
    cap_user_header_t cap_header = &cap_header_data;

    struct __user_cap_data_struct cap_data_data;
    cap_user_data_t cap_data = &cap_data_data;

    cap_header->pid = getpid();
    cap_header->version = _LINUX_CAPABILITY_VERSION_1;

    if (capget(cap_header, cap_data) < 0) {
        perror("Failed capget");
        exit(1);
    }
    if(flag==1) printf("[old]");
    else printf("[new]");
    printf("Cap data permitted: 0x%x, effective: 0x%x, inheritable:0x%xn\n", 
        cap_data->permitted, 
        cap_data->effective,
        cap_data->inheritable);
    return;
}

void init_seccomp(){
    scmp_filter_ctx scmp=seccomp_init(SCMP_ACT_ALLOW);

    if(!scmp){
        perror("failed to initialize libseccomp\n");
        return;
    }

    if(seccomp_rule_add(scmp,SCMP_ACT_KILL,SCMP_SYS(unshare),0)<0){
        perror("seccomp_rule_add_fail");
        return;

    }

    if(seccomp_rule_add(scmp,SCMP_ACT_KILL,SCMP_SYS(setns),0)<0){
        perror("seccomp_rule_add_fail");
        return;
    }


    if(seccomp_load(scmp)!=0){
        perror("failed to load the filter in the kernnel\n");
        return;
    }

}