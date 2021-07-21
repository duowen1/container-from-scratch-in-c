[toc]
# Namespaces

通过Namespace对沙箱进行资源的隔离

## PID Namespace

提供隔离的进程环境，沙箱中运行的进程无法观测到沙箱外的进程。

### Mount `/proc` peseudo file system
`ps`命令依赖于`/proc`，所以需要挂载`/proc`伪文件系统。挂载后通过`ps aux`命令可以查看到沙箱中的进程。

**验证**：在沙箱内执行`ps aux`命令，无法看到沙箱外的进程。

## UTS Namespace

提供主机名`nodename`、网络名`domainname`的隔离

## Mount Namespace

Mount Namespace提供挂载点的隔离。

大部分沙箱程序选择使用`chroot`系统调用进行进程的隔离，从适用性的角度上讲，`chroot`更容易使用，但是从安全的角度上来说，`chroot`面临更多的问题，例如通过传递文件描述符以绕过`chroot`的限制。通过阅读Docker的源码，对Docker启动容器的过程进行分析，这里采用了和Docker相同的`Mount Namespace`+`pivot_root`的方式进行文件系统的隔离。

### pivot_root

- 改变调用进程所在的mount namespace中的根挂载点
- 在实现的时候参考了docker的源码，使用到了一个没有文档化的功能。

`pivot_root(".",".")`将old_root和`/proc/self/cwd`绑定，这样umount会更加容易，无需创建子目录。注意需要`umount(.)`然后进行`chdir("/")`。

### chroot

`chroot`改变调用进程及其子进程的根目录`/`，也是容器技术最早的原型。但是针对于chroot的逃逸案例层出不穷，从安全性的角度考虑，`pivot_root`是更好的选择。

### UnionFS

由于没有实现镜像的功能，为了能让根文件系统进行复用，所以采用了联合文件系统。沙箱内的进程无法直接对根文件系统进行修改，其所有修改会保存在upper层中。

Docker镜像也是通过联合文件系统进行工作的，通过联合文件系统减小镜像的体积。

## IPC Namespace

隔离跨进程通信，不在同一IPC Namespace中的进程不能通过**消息队列**、**共享内存**和**信号量**的方式进行跨进程通信。IPC namespace**没有实现**对所有跨进程通信的方式进行隔离。

## Net Namespace
隔离网络资源，包括网卡、网络栈等。这里网络功能的实现参考了Docker的设计思想。沙箱的网络处于私有的网段中，通过host上的网桥与host进行网络通信。

### Net Bridge
网桥，可以理解为软件版的路由器。

### NAT

分为SNAT和DNAT，使得一个公有IP地址可以支持多个网络设备进行网络通信。

**连接外网：**暂不支持，SNAT和DNAT配置仍有问题

### 验证

1. 沙箱内拥有单独的ip地址，且和host不在同一网段下；
2. 沙箱可以和host进行通信。

## User Namespace
User Namespace是仅在Rootless mode下启用。使用Rootless mode可以显著提高沙箱的安全性。启用User Namespace后沙箱不需要root权限启动，沙箱里的进程会自动获取该User Namespace中的所有Capabilities。

# Control Groups

通过CGroup实现对沙箱资源使用的限制，可以限制沙箱的进程无限消耗CPU和内存等资源，避免造成拒绝服务攻击。这里使用的cgroup版本是CGroupV1。

## cpu sub-system

限制沙箱中的进程所能使用的CPU的最大比例。

## cpuacct sub-system

## cpuset sub-system

限制沙箱中的进程运行在某个特定的CPU核心上

## memory sub-system

限制沙箱中使用的内存数量

### swap

禁用了swap内存

## freezer sub-system

freezer子系统主要为了支持pause命令的实现，执行pause命令会导致沙箱中所有的进程被挂起。

# Capabilities
将系统的root权限进行拆分，通过设置bound的方式限制了沙箱中的所有进程不能获取到特定的Capabilities。执行需要被禁用的Capability的操作的进程会被终止并提示权限不足。被禁用的Capabilities list参考了docker默认drop的Capabilities列表。

# Seccomp
通过filter模式的seccomp限制沙箱中的进程可以进行的系统调用，沙箱中的进程试图执行黑名单中的系统调用时会被终止。被禁用的系统调用list参考了docker默认加载的seccomp配置文件。

# Code Structure
```
$tree .
.
├── detail.md
├── Makefile
├── Readme.md
├── src
│   ├── cap.c
│   ├── cap.h
│   ├── cgroup.c
│   ├── cgroup.h
│   ├── comp.c
│   ├── comp.h
│   ├── container.c
│   ├── container.h
│   ├── run.c
│   ├── run.h
│   ├── utils.c
│   ├── utils.h
│   ├── exec.c
│   ├── exec.h
│   └── Makefile
└── test
    ├── cgroup_test
    │   ├── cpu_cgroup
    │   │   ├── cpu-hungry.c
    │   │   └── Makefile
    │   └── mem_cgroup
    │       ├── Makefile
    │       └── mem_alloc.c
    └── namespace_test
        └── net
            ├── client.c
            ├── Makefile
            └── server.c

7 directories, 19 files
```

# TODO
- [] 通过正确配置NAT以连接互联网
- [] IPC Namespace验证程序
- [] `container cp`、`container ps`、`container exec`、`container pause`命令实现
- [] 容器降权运行
- [] 沙箱内数据的持久化存储
- [] 限制沙箱向硬盘中写入数据的数量
- [] 支持参数的可定制化