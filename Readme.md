- [Namespaces](#namespaces)
  * [PID Namespace](#pid-namespace)
  * [UTS Namespace](#uts-namespace)
  * [Mount Namespace](#mount-namespace)
  * [IPC Namespace](#ipc-namespace)
  * [Net Namespace](#net-namespace)
  * [Time Namespace](#time-namespace)
  * [User Namespace](#user-namespace)
- [Control Groups](#control-groups)
  * [cpu sub-system](#cpu-sub-system)
  * [cpuacct sub-system](#cpuacct-sub-system)
  * [cpuset sub-system](#cpuset-sub-system)
  * [memory sub-system](#memory-sub-system)
  * [freezer sub-system](#freezer-sub-system)
- [Capabilities](#capabilities)
- [Seccomp](#seccomp)
- [Code Structure](#code-structure)

# Namespaces

## PID Namespace

### Mount `/proc` peseudo file system 

## UTS Namespace

## Mount Namespace

### pivot_root

### chroot

## IPC Namespace

## Net Namespace

### Net Bridge

### NAT

## Time Namespace

## User Namespace
User Namespace是仅在Rootless mode下启用

# Control Groups

## Cgroup V1

### cpu sub-system

### cpuacct sub-system

### cpuset sub-system

### memory sub-system

#### swap

## freezer sub-system

## Cgroup V2

**为什么要使用Cgroup v2**
- 和V1相比，Cgroup V2的设计思路更加直接，尤其是实现了Buffer IO的限制，这也是采用Cgroup V2最大的殷殷。

**如何开启Cgroup V2**
- 修改内核引导参数，在`/etc/default/grub`中添加：

```
GRUB_CMDlINE_LINUX="... systemd.unified_group_hierachy=1"
```

然后执行：
```
sudo update-grub
sudo reboot
```

### IO sub-system

Cgroup V1中对于IO的速率没有进行Buffer IO的限制，在生产环境中这种限制没有任何意义。在Cgroup V2中解决了这个问题


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

# Some Questions
1. 外网的连通性

目前可以完成和宿主机的通信

2. 对文件系统写入的限制


# Reference

- [CS 695:Virtualization and Cloud Computing](https://www.cse.iitb.ac.in/~cs695/)
- [详解Cgroup v2](https://zorrozou.github.io/docs/%E8%AF%A6%E8%A7%A3Cgroup%20V2.html)
- [Cgroup v2官方文档](https://www.kernel.org/doc/html/v5.7/admin-guide/cgroup-v2.html)