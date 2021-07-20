[toc]
# Namespaces

## PID Namespace

### Mount `/proc` peseudo file system 

## UTS Namespace

## Mount Namespace

### pivot_root

- 改变调用进程所在的mount namespace中的根挂载点
- 在实现的时候参考了docker的源码，使用到了一个没有文档化的功能。

`pivot_root(".",".")`将old_root和`/proc/self/cwd`绑定，这样umount会更加容易，无需创建子目录。注意需要`umount(.)`然后进行`chdir("/")`。

### chroot

chroot改变调用进程及其子进程的`/`

## IPC Namespace

## Net Namespace

### Net Bridge

### NAT

## Time Namespace

## User Namespace
User Namespace是仅在Rootless mode下启用


# Control Groups

## cpu sub-system

## cpuacct sub-system

## cpuset sub-system

## memory sub-system

### swap

## freezer sub-system

# Capabilities

# Seccomp

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