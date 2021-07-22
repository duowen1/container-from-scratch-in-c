#include "cgroup.h"

enum cgroup_type{
    Cpu,
    Cpuset,
    Memory,
    Freezer
};

const char *CgroupType[] = {
    "cpu",
    "cpuset",
    "memory",
    "freezer"
};

void cgroup(pid_t pid, char * name){
    init_cpu_cgroup(pid, name);
    init_cpuset_cgroup(pid, name); 
    init_memory_cgroup(pid, name);
    init_freezer_cgroup(pid, name);
}

int init_cpu_cgroup(pid_t pid, char * name){
    char buffer[100];
    cgroup_mkdir_name(Cpu, name);
    snprintf(buffer, sizeof(buffer), "%d", 100000);
    cgroup_fd_operation(Cpu, name, "cpu.cfs_quota_us", buffer);
    snprintf(buffer, sizeof(buffer), "%d", pid);
    cgroup_fd_operation(Cpu, name, "cgroup.procs", buffer);
    return 0;
}

int init_cpuset_cgroup(pid_t pid, char * name){
    cgroup_mkdir_name(Cpuset, name);
    cgroup_fd_operation(Cpuset, name, "cpuset.cpus", "2-3");
    cgroup_fd_operation(Cpuset, name, "cpuset.mems", "0");
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "%d", pid);
    cgroup_fd_operation(Cpuset, name, "cgroup.procs", "pid");
    return 0;
}

int init_memory_cgroup(pid_t pid, char * name){
    cgroup_mkdir_name(Memory, name);
    char buffer[100];
    snprintf(buffer,sizeof(buffer),"%d", MEMORY_LIMIT);
    cgroup_fd_operation(Memory, name, "memory.limit_in_bytes", buffer);
    cgroup_fd_operation(Memory, name, "memory.oom_control", "0");
    cgroup_fd_operation(Memory, name, "memory.swappiness", "0");
    snprintf(buffer, sizeof(buffer), "%d", pid);
    cgroup_fd_operation(Memory, name, "cgroup.procs", buffer);
    return 0;
}

int init_freezer_cgroup(pid_t pid, char * name){
    char buffer[100];
    cgroup_mkdir_name(Freezer, name);
    snprintf(buffer, sizeof(buffer), "%d", pid);
    return cgroup_fd_operation(Freezer, name, "cgroup.procs", buffer);;
}

int freeze(char * name, int direction){
    if(direction == FREEZE){
        return cgroup_fd_operation(Freezer, name, "freezer.state", "FROZEN");
    }else{
        return cgroup_fd_operation(Freezer, name, "freezer.state", "THAWED");
    }
}

int cgroup_mkdir_name(int type, char * container_name){
    char dirpath[100];
    snprintf(dirpath, sizeof(dirpath), "/sys/fs/cgroup/%s/%s", CgroupType[type], container_name);
    return mkdir(dirpath,0755);
}

int cgroup_fd_operation(int type, char * container_name, char * filename, char *towrite){
    char filepath[100];
    snprintf(filepath,sizeof(filepath),"/sys/fs/cgroup/%s/%s/%s",CgroupType[type],container_name,filename);
    FILE * fd = NULL;
    fd = fopen(filepath,"w");
    if(fd == NULL){
        perror("Open file fail");
        exit(0);
    }
    fputs(towrite,fd);
    fclose(fd);
    return 0;
}