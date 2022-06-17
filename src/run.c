#include "run.h"

static char * init_unionfs(void *);
static int init_network();
static int childfunction(void*);
static int setup_hostname(char *);
static int setup_rootfs();
static int setup_proc();
static int setup_network();
static int clean_up(char *);

int container_run(char *argv[]){
    checkroot();

    char * container_name = init_unionfs(argv);
    argv[3] = container_name;

    init_network();

    printf("args:\n%s\n%s\n%s\n%s\n",argv[0],argv[1],argv[2],argv[3]);
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
        
        system("ip link add veth0 type veth peer name veth1");
        system("brctl addif br0 veth0");//create a new bridge
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
        }
        
        // system("ip addr add 172.10.0.1/24 dev veth0");

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

static int init_network(){
    char filepath[] = "/proc/sys/net/ipv4/ip_forward";

    system("brctl addbr br0");
    system("ip addr add 172.10.0.1/24 broadcast 172.10.0.255 dev br0");
    system("ip link set br0 up");

    //echo 1 > /proc/sys/net/ipv4/ip_forward
    FILE * fd = NULL;
    fd = fopen(filepath, "w");
    if(fd == NULL){
        perror("Open file fail");
        exit(0);
    }
    fputs("1", fd);
    fclose(fd);

    //SNAT
    system("iptables -P FORWARD ACCEPT");
    system("iptables -t filter -A FORWARD -i br0 ! -o br0 -j ACCEPT");
    system("iptables -t nat -A POSTROUTING ! -o br0 -s 172.10.0.0/24 -j MASQUERADE");

    //DNAT
    system("iptables -t nat -A PREROUTING -p tcp -m tcp --dport 30001 ! -i br0 -j DNAT --to-destination 172.10.0.201:5050");
    return 0;
}

//the function which new process executed
static int childfunction(void *arg){
    int res;

    setup_hostname(((char **)arg)[2]);
    printf("[SANDBOX]Set hostname success\n");

    sleep(1);//wait the parent namespace to set network
    
    //setup container network
    
    res = setup_network();
    if(res){
        printf("[SANDBOX]Init network wrong\n");
    }else{
        printf("[SANDBOX]Init network success\n");
    }
    
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

    //setresuid(1,1,1);
    //setresgid(1,1,1);

    res = execv("/bin/bash", &args);
    perror("[SANDBOX]: res \n");

    return 0;
}

static char * init_unionfs(void * arg){
    int ret;
    char * rootfs = ((char **) arg)[3];

    ret = chdir(rootfs);
    if(ret == -1){
        perror("wrong rootfs path");
        exit(-1);
    }

    char * container_name = generate_random_string(CONTAINER_NAME_LEN);
    mkdir(container_name,0755);
    chdir(container_name);

    mkdir("merge",0755);
    mkdir("upper",0755);
    mkdir("work",0755);
    ret = mount("overlay","./merge","overlay",0,"lowerdir=../rootfs,upperdir=./upper,workdir=./work");
    if(ret != 0){
        perror("res");
        exit(1);
    }
    // exit(1);
    return container_name;
}

static int setup_hostname(char * hostname){
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

static int setup_rootfs(){
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
        perror("[rootfs] chdir wrong");
        exit(1);
    }
        
    int oldroot_fd = open("/", O_DIRECTORY | O_RDONLY, 0);
    
    int newroot_fd = open(".", O_DIRECTORY | O_RDONLY , 0);
    if(newroot_fd == -1){
        perror("open rootfs fail");
        printf("%s\n", rootfs);
        exit(1);
    }

    res = fchdir(newroot_fd);
    if(res!=0){
        perror("[newroot_fd] fchdir wrong");
        exit(1);
    }

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

static int setup_proc(){
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

static int setup_network(){
    int res;
    res = system("ip link set lo up");//turn on loop back address
    system("ip link set veth1 up");//turn on the network advice
    system("ip addr add 172.10.0.201/24 brintoadcast 172.10.0.255 dev veth1");//set the ip address on device
    
    //we use route command, which is not existed in the new filesystem, so we must execute this commond before "chroot"
    system("route add default gw 172.10.0.1");//add the net gate address to iptables

    return res;
}

static int clean_up(char * name){
    umount("merge");
    rmdir("merge");
    system("rm -r upper work");//I am lazy
    chdir("../");
    rmdir(name);
    free(name);
}