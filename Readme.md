# 要求
编写C/C++程序，在一个类似于容器的隔离环境中弹出命令行shell，具体要求如下：
- shell必须在独立的network、mount、PID、UTS namespace中运行；
- 将根文件系统作为参数传递给程序，程序需要在根文件系统的根目录下运行；
- 将主机名作为参数传递给程序，执行hostname命令需要展示出主机名；
- 执行ps命令必须展示PID namespace下的进程列表；
- 容器必须拥有独立的IP地址，并且通过运行一个C/S架构程序展示程序的网络连通性，服务端需要在容器中启动、客户端运行在父namespace中，服务端程序需要提前放置在根文件系统中；
- 针对某一项资源（CPU或者内存）进行限制，通过shell执行某一资源需求型程序以展示限制效果；
- 不能调用已经存在的容器运行时例如LXC。

# 背景知识

## Namespace
通过不同的Namespace完成对内核资源的隔离

### 系统调用接口
Linux内核提供了三个有关于Namespace的系统调用，包括```clone```、```unshare```、```setns```:
#### clone
创建进程，使创造出的进程运行在指定的namespace中：

**函数原型**
```c
int clone(int (*child_func)(void *), void *child_stack, int flags, void *arg);
```

- clone与fork区别：更加通用，本质上调用了同样的系统调用
- clone与exec*函数：创建新的进程，而不是将新的进程加入的自己的地址空间中

##### flag

Namespace | flag
---|---
mount | CLONE_NEWNS
UTS | CLONE_NEWUTS
PID | CLONE_NEWPID
NETWORK | CLONE_NEWNET
USER | CLONE_NEWUSER
IPC | CLONE_NEWIPC
Control Group | CLONE_NEWCGROUP


#### unshare
创建新的namespace，并且将调用进程放置进入到namespace中

**函数原型**
```c
int unshare(int flags);
```

#### setns
使调用进程加入到一个已经存在的namespace中

**函数原型**
```c
int setns(int fd, int nstype);
```

##### ```/proc/PID/ns```文件
通过```/proc/PID/ns```目录展示了该进程所属的namespace，每个namespace有一个符号链接文件。

```bash
xsw@ubuntu:~$ sudo ls -l /proc/4338/ns
total 0
lrwxrwxrwx 1 root root 0 Dec  4 14:41 cgroup -> 'cgroup:[4026531835]'
lrwxrwxrwx 1 root root 0 Dec  4 14:41 ipc -> 'ipc:[4026531839]'
lrwxrwxrwx 1 root root 0 Dec  4 14:41 mnt -> 'mnt:[4026531840]'
lrwxrwxrwx 1 root root 0 Dec  4 14:41 net -> 'net:[4026532000]'
lrwxrwxrwx 1 root root 0 Dec  4 14:41 pid -> 'pid:[4026531836]'
lrwxrwxrwx 1 root root 0 Dec  4 14:41 pid_for_children -> 'pid:[4026531836]'
lrwxrwxrwx 1 root root 0 Dec  4 14:41 user -> 'user:[4026531837]'
lrwxrwxrwx 1 root root 0 Dec  4 14:41 uts -> 'uts:[4026531838]'
xsw@ubuntu:~$ ps aux | grep bash
xsw        2881  0.0  0.1  22700  3324 pts/0    Ss   14:31   0:00 bash
xsw        4313  0.0  0.2  22708  4732 pts/1    Ss   14:39   0:00 /bin/bash
root       4339  0.0  0.1  20260  3204 pts/1    S+   14:39   0:00 [bash]
xsw        4463  0.0  0.0  14424  1088 pts/0    S+   14:43   0:00 grep --color=auto bash
xsw@ubuntu:~$ sudo ls -l /proc/4339/ns
total 0
lrwxrwxrwx 1 root root 0 Dec  4 14:44 cgroup -> 'cgroup:[4026531835]'
lrwxrwxrwx 1 root root 0 Dec  4 14:44 ipc -> 'ipc:[4026531839]'
lrwxrwxrwx 1 root root 0 Dec  4 14:44 mnt -> 'mnt:[4026532630]'
lrwxrwxrwx 1 root root 0 Dec  4 14:44 net -> 'net:[4026532634]'
lrwxrwxrwx 1 root root 0 Dec  4 14:44 pid -> 'pid:[4026532632]'
lrwxrwxrwx 1 root root 0 Dec  4 14:44 pid_for_children -> 'pid:[4026532632]'
lrwxrwxrwx 1 root root 0 Dec  4 14:44 user -> 'user:[4026531837]'
lrwxrwxrwx 1 root root 0 Dec  4 14:44 uts -> 'uts:[4026532631]'

```

fd参数是```/proc/PID/ns```的文件描述符

### 类型 

#### UTS
隔离主机名、节点名

可以通过```sethostname```以及```setdomainname```对namespace中的主机名和节点名进行隔离

#### PID
隔离进程号
PID namespace中第一个进程是init进程，pid为1

##### 孤儿进程
父进程退出，而其子进程仍在运行，子进程将成为孤儿进程。孤儿进程会被init进程“收养”，即init进程成为孤儿进程的父进程。

##### 僵尸进程
僵尸进程指的是目前为僵尸态的进程。进程使用fork创建子进程，子进程退出，父进程没有调用wait或者waitpid系统调用，则子进程的进程描述符仍保留在系统中，进程所占用的其他系统资源已经被释放。

- **思考**：为什么Linux系统要设计僵尸进程？
- **思考**：僵尸进程有什么危害？



#### mount
隔离每个Namespace实例中的进程所看到的文件系统列表，有三个文件与其相关。

```
cat /proc/PID/mounts #所有挂载在当前Namespace中的文件系统
cat /proc/PID/mountinfo #挂载文件系统的属性信息
cat /proc/PID/mountstatus #文件设备的统计信息，包括挂载文件名称、文件系统类型、挂载位置等
```

所有位于同一个Mount Namespace中的进程在这三个文件中看到的内容相同

#### user
隔离用户和组，在不使用的user namespace的情况下，namespace中的进程将以root权限运行。

#### IPC
隔离消息队列

#### cgroup
隔离cgroup

## Cgroup
Linux内核提供的一种可以限制单个进程或者多个进程所使用资源的机制，可以对CPU、内存等资源实现精细化控制。

### 概念
- cgroup
- subsystem：为每种可以控制的资源（cpu、memory、blkio等）定义了一个子系统
- hierarchy：层级结构

![cgroup层级结构示意图](https://awps-assets.meituan.net/mit-x/blog-images-bundle-2015/b3270d03.png)

- 树形结构，每棵树是一个层级结构
- 每个节点是一个结构体
- 每个层级结构可以attach一个或者多个cgroup子系统
- 每个层级结构可以对其attach的cgroup子系统进行资源限制，每一个子系统只能被attach到一个cpu层级结构中
- 可以把进程加入到某一个节点的控制任务列表中，同一进程可以加入到不同层级结构的节点中
- 通过在挂载点下建立文件夹的方式创建cgroups层级结构中的节点

```
/sys/fs/cgroup/[subsystem]
```

### CPU子系统
```cpu.shares```：默认通常为1024，根据shares的比例值获取cpu时间
```cpu.cfs_period_us```：完全公平调度器的调整时间配额周期，一般值为100000
```cpu.cfs_quota_us```：完全公平调度器的周期当中可以占用的时间，默认为-1，即不受限制
```cpu.stat```：统计值

```
top - 11:07:24 up 7 min,  1 user,  load average: 0.47, 0.31, 0.19
Tasks: 337 total,   1 running, 252 sleeping,   0 stopped,   0 zombie
%Cpu(s):  0.1 us,  0.8 sy,  0.0 ni, 99.1 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
KiB Mem :  2006960 total,    85712 free,  1368632 used,   552616 buff/cache
KiB Swap:  1942896 total,  1919332 free,    23564 used.   436980 avail Mem
```

### Memeoy子系统

### Cgroup V2

## chroot
切换系统的根位置，应用程序在文件层面上被限制在根目录下。

## capability
细粒度的权限管理机制，Linux将超级用户不同单元的权限分开，可以单独开启和禁止。内核在检查进程是否具有某项权限时，不在检查该进程是特权进程还是非特权进程，而是检查该进程是否具有进行该操作的capability。

```
cat /usr/include/linux/capability.h

……
/* Allow configuration of the secure attention key */
/* Allow administration of the random device */
/* Allow examination and configuration of disk quotas */
/* Allow setting the domainname */
/* Allow setting the hostname */
/* Allow calling bdflush() */
/* Allow mount() and umount(), setting up new smb connection */
/* Allow some autofs root ioctls */
/* Allow nfsservctl */
/* Allow VM86_REQUEST_IRQ */
/* Allow to read/write pci config on alpha */
/* Allow irix_prctl on mips (setstacksize) */
/* Allow flushing all cache on m68k (sys_cacheflush) */
/* Allow removing semaphores */
/* Used instead of CAP_CHOWN to "chown" IPC message queues, semaphores
   and shared memory */
/* Allow locking/unlocking of shared memory segment */
/* Allow turning swap on/off */
/* Allow forged pids on socket credentials passing */
/* Allow setting readahead and flushing buffers on block devices */
/* Allow setting geometry in floppy driver */
/* Allow turning DMA on/off in xd driver */
/* Allow administration of md devices (mostly the above, but some
   extra ioctls) */
/* Allow tuning the ide driver */
/* Allow access to the nvram device */
/* Allow administration of apm_bios, serial and bttv (TV) device */
/* Allow manufacturer commands in isdn CAPI support driver */
/* Allow reading non-standardized portions of pci configuration space */
/* Allow DDI debug ioctl on sbpcd driver */
/* Allow setting up serial ports */
/* Allow sending raw qic-117 commands */
/* Allow enabling/disabling tagged queuing on SCSI controllers and sending
   arbitrary SCSI commands */
/* Allow setting encryption key on loopback filesystem */
/* Allow setting zone reclaim policy */

#define CAP_SYS_ADMIN        21
……
```

## seccomp
一种限制系统调用的方式，通过设置seccomp，可以限制进程执行特定的系统调用。

### 早期seccomp
早期的seccomp只能允许```read```、```write```、```_exit```、```sigreturn```四个系统调用，当调用这四个系统调用之外时进程会被kill并输出```Kill```

### 基于BPF的seccomp
#### BPF
[伯克利包过滤器](https://zh.m.wikipedia.org/zh-hans/BPF)

在Linux 3.5后引入基于BPF的可定制的系统调用过滤功能
```c
#include <stdio.h>
#include <unistd.h>
#include <seccomp.h>
int main() {
  scmp_filter_ctx ctx;
  ctx = seccomp_init(SCMP_ACT_KILL);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(rt_sigreturn), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(dup2), 2,
                        SCMP_A0(SCMP_CMP_EQ, 1),
                        SCMP_A1(SCMP_CMP_EQ, 2));
  seccomp_load(ctx);
  dup2(1, 2);
  dup2(2, 42);
  return 0;
}

```

# UTS namespace
参考Linux Kernel文档编写程序：
```c
static int childFunc(void *arg){
    struct utsname uts;
    if (sethostname(arg, strlen(arg)) == -1) errExit("sethostname");
    if (uname(&uts) == -1) errExit("uname");
    printf("uts.nodename in child:  %s\n", uts.nodename);
    sleep(100);
    return 0;
}

#define STACK_SIZE (1024 * 1024)
static char child_stack[STACK_SIZE];

int main(int argc, char *argv[]){
    pid_t child_pid;
    struct utsname uts;
    child_pid = clone(childFunc, 
                    child_stack + STACK_SIZE, 
                    CLONE_NEWUTS | SIGCHLD, argv[1]);
    if (child_pid == -1) errExit("clone");
    printf("PID of child created by clone() is %ld\n", (long) child_pid);
    sleep(1);
    if (uname(&uts) == -1) errExit("uname");
    printf("uts.nodename in parent: %s\n", uts.nodename);
    if (waitpid(child_pid, NULL, 0) == -1) errExit("waitpid");
    printf("child has terminated\n");
    exit(EXIT_SUCCESS);
}
```

# PID namespace

## 挂载proc文件系统
有些命令例如```ps```、```top```命令会读取```/proc```目录下的文件，所以在未挂载```proc```文件系统前，```ps```和```top```命令会展示出父namespace中的进程

```c
char *mount_point = "proc";
if(mount_point != NULL){
    mkdir(mount_point,0555);
    if(mount("proc",mount_point,"proc",0,NULL)==-1){
        perror("Mount fail");
        exit(1);
    }else{
        printf("Mounting procfs as %s\n",mount_point);
    }
}
```

# Network namespace
通过veth设备对的方式完成通信

**host**
```
ip link add veth0 type veth peer name veth1
ip link set veth1 netns [pid]
ip link set veth0 up
ip addr [ip/mask] dev veth0
```

**container**
```
ip link set lo up
ip link set veth1 up
ip addr add [ip/mask] dev veth1
route add default gw [net gate] veth1
```

# chroot(mount namespace)
## 文件系统
下载一个制作好的root filesystem

[rootfs](https://github-production-release-asset-2e65be.s3.amazonaws.com/71006166/5767750c-92cc-11e6-98bd-64be9b36cd82?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAIWNJYAX4CSVEH53A%2F20201203%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20201203T065815Z&X-Amz-Expires=300&X-Amz-Signature=97b774885f1f49c3a5b4c0d2d5ba0134831167dee5fa0123418809ef776e70fd&X-Amz-SignedHeaders=host&actor_id=38399831&key_id=0&repo_id=71006166&response-content-disposition=attachment%3B%20filename%3Drootfs.tar.gz&response-content-type=application%2Foctet-stream)

## 根文件目录
```
chroot(rootfs)
chdir("/")
```

# cpu cgroup
## cpu-hungry进程
编写死循环程序以消耗CPU资源

```c
void *doSomeThing(void *arg){
	int i = 0;
	pthread_t id = pthread_self();
	while (1) {
		int sum;
		sum += 1;
	}
	return NULL;
}
int main(int argc, char *argv[]){
	int i;
	int total;
	pthread_attr_t tattr;
	int err;
	int stack_size = (20 * 1024 * 1024);
	total = atoi(argv[1]);
	pthread_attr_init(&tattr);
	pthread_attr_setstacksize(&tattr, stack_size);
	for (i = 0; i < total; i++) {
		pthread_t tid;
		pthread_create(&tid, &tattr, &doSomeThing, NULL);
	}
	usleep(1000000);
	printf("All threads are created\n");
	usleep(1000000000);
	return EXIT_SUCCESS;
}
```

## 设置cpu cgroup
**host**
```
void cgroup(pid_t pid){
    FILE *cgroup_cpu_fd = NULL;
    FILE *cgroup_procs_fd = NULL;
    mkdir("/sys/fs/cgroup/cpu/group1/",0755);

    cgroup_cpu_fd=fopen("/sys/fs/cgroup/cpu/group1/cpu.cfs_quota_us","w");
    fprintf(cgroup_cpu_fd,"%d",100000);
    fclose(cgroup_cpu_fd);

    cgroup_procs_fd=fopen("/sys/fs/cgroup/cpu/group1/cgroup.procs","w");
    fprintf(cgroup_procs_fd,"%d",pid);
    fclose(cgroup_procs_fd);
    printf("Creat cgroup success\n");
    return;
}
```


# 结果

```
sudo ./container mycontainer ../rootfs/
```

## UTS

```bash
root@mycontainer:/# uname -n
mycontainer
```

## PID
ps命令只展示了当前namespace中的进程，其中pid为1的进程就是这个namespace中的init进程。

```
root@mycontainer:/# ps
   PID TTY          TIME CMD
     1 ?        00:00:00 bash
    10 ?        00:00:00 ps
```

## 网络
**container**
```
root@mycontainer:/# ip addr
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host 
       valid_lft forever preferred_lft forever
4: veth1@if5: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default qlen 1000
    link/ether 42:9e:02:24:7f:44 brd ff:ff:ff:ff:ff:ff
    inet 192.168.31.10/24 scope global veth1
       valid_lft forever preferred_lft forever
    inet6 fe80::409e:2ff:fe24:7f44/64 scope link 
       valid_lft forever preferred_lft forever
```

**host**
```
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host 
       valid_lft forever preferred_lft forever
2: ens33: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UP group default qlen 1000
    link/ether 00:0c:29:ed:fb:89 brd ff:ff:ff:ff:ff:ff
    inet 192.168.31.56/24 brd 192.168.31.255 scope global dynamic noprefixroute ens33
       valid_lft 27682sec preferred_lft 27682sec
    inet6 fe80::a7b0:d244:68fa:495b/64 scope link noprefixroute 
       valid_lft forever preferred_lft forever
3: docker0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc noqueue state DOWN group default 
    link/ether 02:42:9f:75:31:04 brd ff:ff:ff:ff:ff:ff
    inet 172.17.0.1/16 brd 172.17.255.255 scope global docker0
       valid_lft forever preferred_lft forever
5: veth0@if4: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default qlen 1000
    link/ether da:5e:4f:3e:c3:6b brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 192.168.31.1/24 scope global veth0
       valid_lft forever preferred_lft forever
    inet6 fe80::d85e:4fff:fe3e:c36b/64 scope link 
       valid_lft forever preferred_lft forever
```

### 简单的套接字通信
#### 服务端
```c
sockfd=socket(AF_INET,SOCK_STREAM,0);
server.sin_family=AF_INET;
server.sin_addr.s_addr=htonl(INADDR_ANY);
server.sin_port=htons(port);
bind(sockfd,(struct sockaddr *)(&server),sizeof(struct sockaddr));
listen(sockfd,1);
new_fd=accept(sockfd,(struct sockaddr*)(&client),&sin_size);
write(new_fd,"hello from the other side",strlen("hello from the other side")); 
close(new_fd);
close(sockfd);
```

#### 客户端

```c
sockfd = socket(AF_INET , SOCK_STREAM , 0)
server.sin_family = AF_INET;
inet_aton(serverip , &server.sin_addr)
server.sin_port = htons(port);
connect(sockfd , (struct sockaddr *)(&server) , sizeof(struct sockaddr));
read(sockfd , buffer , 1024);
close(sockfd);
```

**container**
```
root@mycontainer:/# ./server
```

**host**
```
xsw@ubuntu:~/Desktop/container$ ./client
Received 25 bytes :hello from the other side
```

## 文件系统隔离

```
root@mycontainer:/# ls
bin  boot  dev  etc  home  lib  lib64  media  mnt  opt  proc  root  run  sbin  server  srv  sys  threads-cpu  tmp  usr  var
```

## 系统资源限制
当```cpu.cfs_quoto_us=100000```
```
top - 11:23:24 up 23 min,  1 user,  load average: 0.63, 0.34, 0.19
Tasks: 353 total,   1 running, 271 sleeping,   0 stopped,   0 zombie
%Cpu(s): 25.5 us,  0.2 sy,  0.0 ni, 74.1 id,  0.0 wa,  0.0 hi,  0.2 si,  0.0 st
KiB Mem :  2006960 total,   122688 free,  1579652 used,   304620 buff/cache
KiB Swap:  1942896 total,  1288492 free,   654404 used.   222176 avail Mem 

   PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND                                  
  4205 root      20   0   47344    756    672 S  99.7  0.0   0:30.39 threads-cpu  
```

当```cpu.cfs_quoto_us=80000```
```
top - 11:26:26 up 26 min,  1 user,  load average: 0.94, 0.61, 0.32
Tasks: 364 total,   1 running, 279 sleeping,   0 stopped,   1 zombie
%Cpu(s): 20.7 us,  0.2 sy,  0.0 ni, 79.1 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
KiB Mem :  2006960 total,   187324 free,  1487044 used,   332592 buff/cache
KiB Swap:  1942896 total,  1141616 free,   801280 used.   272960 avail Mem 

   PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND                                  
  4369 root      20   0   47344    748    660 S  80.1  0.0   0:13.16 threads-cpu
```