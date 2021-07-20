#include "container.h"

//the function which new process executed
static int childfunction(void *arg){
    int res;

    setup_hostname(((char **)arg)[2]);
    printf("[SANDBOX]Set hostname success\n");

    sleep(1);//wait the parent namespace to set network
    
    //setup container network
    /*
    res = setup_network();
    if(res){
        printf("[SANDBOX]Init network wrong\n");
    }else{
        printf("[SANDBOX]Init network success\n");
    }
    */

    setup_rootfs(((char **)arg)[3]);
    printf("[SANDBOX]Init rootfs success\n");

    setup_proc();
    printf("[SANDBOX]Mount proc pesudo file system success\n");

    init_capability();
    printf("[SANDBOX]Init capability success, show:\n");
    list_capability(2);

    res = init_comp();
    printf("[SANDBOX]Init seccomp success\n");

    char * args=NULL;
    execv("/bin/bash", &args);
    return 0;
}

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

    if(!strcmp(argv[1],"help")){//如果是help命令
        container_help();
    }

    return 0;
}


int setup_hostname(char * hostname){
    int res = sethostname(hostname,strlen(hostname));
    if(res){
        perror("[SANDBOX]sethostname fail:");
        exit(1);
    }
    res = setdomainname(hostname,strlen(hostname));
    if(res){
        perror("[SANDBOX]setdomainname fail:");
        exit(1);
    }
    return res;
}

int setup_network(){
    int res;
    res = system("ip link set lo up");//turn on loop back address
    system("ip link set veth1 up");//turn on the network advice
    system("ip addr add 172.10.0.201/24 dev veth1");//set the ip address on device
    //we use route command, which is not existed in the new filesystem, so we must execute this commond before chroot
    system("route add default gw 172.10.0.1");//add the net gate address to iptables

    return res;
}

int setup_rootfs(char * rootfs){
    int res;
    mount("", "/", "", MS_PRIVATE, NULL);
    mount(rootfs, rootfs, "bind", MS_BIND | MS_REC, NULL);
    chdir(rootfs);    
    int oldroot_fd = open("/", O_DIRECTORY | O_RDONLY, 0);
    int newroot_fd = open(rootfs, O_DIRECTORY | O_RDONLY , 0);
    fchdir(newroot_fd);

    res = syscall(SYS_pivot_root, "." ,".");
    if(res!=0){
        perror("pivot_root wrong");
        exit(1);
    }
    
    fchdir(oldroot_fd);
    mount("",".","",MS_SLAVE|MS_REC,NULL);
    res = syscall(SYS_umount2, ".", MNT_DETACH);
    if(res!=0){
        perror("umount2 wrong");
        exit(1);
    }

    chdir("/");

    close(oldroot_fd);
    close(newroot_fd);
    return res;
}

int setup_proc(){
    char *mount_point = "proc";
    if(mount_point != NULL){
        mkdir(mount_point,0555);
        if(mount("proc",mount_point,"proc",0,NULL)==-1){
            perror("Mount fail");
            exit(1);
        }
    }
    return 0;
}

int container_run(char *argv[]){

    checkroot();

    int flag = CLONE_NEWUTS | CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWNET | CLONE_NEWCGROUP;

    list_capability(HOST);
    pid_t child_pid;
    struct utsname uts;

    child_pid = clone(childfunction,child_stack+STACK_SIZE,flag | SIGCHLD,(void*)argv);
    if(child_pid == -1){
        //output error information and exit
        perror("Create process fail");
        exit(1);
    }
    else{
        cgroup(child_pid);//set cgroup rules
        printf("[HOST]Creat sandbox cgroup success\n");
        
        int res;
        /*
        system("ip link add veth0 type veth peer name veth1");
        system("brctl addif br0 veth0");
        res = system("ip link set veth0 up");

        if(res){
            perror("ip linke set veth0 up fail");
            exit(1);
        }

        char cmd[100];
        sprintf(cmd,"ip link set veth1 netns %d",child_pid);
        res = system(cmd);

        if(res){
            perror("ip link set veth1 netns chile_pid fail");
            exit(1);
        }*/
        
        //system("ip addr add 192.168.31.1/24 dev veth0");
        if(uname(&uts)==-1){//print the uts from host
            //output error information and exit
            perror("uname fail: ");
            exit(1);
        }else{
            printf("[HOST]PID returned by clone(): %ld\n",(long)child_pid);
            printf("[HOST]uts.nodename in parent : %s\n",uts.nodename);
        }
        sleep(2);
        if(waitpid(child_pid,NULL,0)==-1){
            //output error information and exit
            perror("child terminate fail:");
            printf("%d\n",errno);
            exit(1);

        }else{
            printf("[HOST]child has terminated\n");
        }
    }

}

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

int container_cp(char * argv[]){//目前仅支持从文件复制到容器内，在没有实现unionfs的情况下感觉没啥意义
    //container cp <file path> <file path>

    char * path1 = argv[2];
    char * path2 = argv[3];

    
    
        
    
    checkroot();
    
    return 0;
}

int container_help(){
    printf("Usage: container <command> [ops]\n");
    return 0;
}

int checkroot(){
    if(getuid()){
        printf("run me as root");
        exit(1);
    }
}