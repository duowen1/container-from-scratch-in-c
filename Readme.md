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