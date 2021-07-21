#include "run.h"

int container_run(char *argv[]){
    checkroot();
    char * container_name = init_unionfs();
    argv[3] = container_name;
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
        cgroup(child_pid,container_name);//set cgroup rules
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
            sleep(1);
            clean_up(container_name);
        }
    }
}


//the function which new process executed
int childfunction(void *arg){
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

    setup_rootfs();
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


char * init_unionfs(){
    chdir("../..");
    char * container_name = generate_random_string(CONTAINER_NAME_LEN);
    mkdir(container_name,0755);
    chdir(container_name);

    mkdir("merge",0755);
    mkdir("upper",0755);
    mkdir("work",0755);
    mount("overlay","./merge","overlay",0,"lowerdir=../rootfs,upperdir=./upper,workdir=./work");
    return container_name;
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

int setup_rootfs(){
    const char rootfs[]="merge";
    int res;
    res = mount("", "/", "", MS_REC | MS_SLAVE, NULL);
    if(res!=0){
        perror("mount private wrong");
        exit(1);
    }
    res = mount(rootfs, rootfs, "bind", MS_BIND | MS_REC, NULL);
    if(res!=0){
        perror("mount bind wrong");
        exit(1);
    }

    res = chdir(rootfs);
    if(res!=0){
        perror("chdir wrong");
        exit(1);
    }    
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

int setup_network(){
    int res;
    res = system("ip link set lo up");//turn on loop back address
    system("ip link set veth1 up");//turn on the network advice
    system("ip addr add 172.10.0.201/24 dev veth1");//set the ip address on device
    //we use route command, which is not existed in the new filesystem, so we must execute this commond before chroot
    system("route add default gw 172.10.0.1");//add the net gate address to iptables
    return res;
}

int clean_up(char * name){
    umount("merge");
    rmdir("merge");
    system("rm -r upper work");//I am lazy
    chdir("../");
    rmdir(name);
    free(name);
}