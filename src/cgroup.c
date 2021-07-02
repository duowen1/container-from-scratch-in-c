#include "cgroup.h"

void cgroup(pid_t pid){
    init_cpu_cgroup(pid);
    init_cpuset_cgroup(pid); 
}

int init_cpu_cgroup(pid_t pid){
    FILE *cgroup_cpu_fd = NULL;
    FILE *cgroup_procs_fd = NULL;
    mkdir("/sys/fs/cgroup/cpu/group1/",0755);

    cgroup_cpu_fd=fopen("/sys/fs/cgroup/cpu/group1/cpu.cfs_quota_us","w");
    if(cgroup_cpu_fd==NULL){
        perror("[cpu] Open fail fail");
        exit(1);
    }
    fprintf(cgroup_cpu_fd,"%d",100000);
    fclose(cgroup_cpu_fd);

    cgroup_procs_fd=fopen("/sys/fs/cgroup/cpu/group1/cgroup.procs","w");
    if(cgroup_procs_fd==NULL){
        perror("[cpu] Open file fail");
        exit(1);
    }
    fprintf(cgroup_procs_fd,"%d",pid);
    fclose(cgroup_procs_fd);
    return 0;
}

int init_cpuset_cgroup(pid_t pid){
    FILE *cgroup_cpuset_fd = NULL;
    FILE *cgroup_procs_fd = NULL;
    mkdir("/sys/fs/cgroup/cpuset/group1/",0755);

    cgroup_cpuset_fd=fopen("/sys/fs/cgroup/cpuset/group1/cpuset.cpus","w");
    if(cgroup_cpuset_fd==NULL){
        perror("[cpuset]Open fail fail");
        exit(1);
    }
    fprintf(cgroup_cpuset_fd,"%d-%d",2,3);
    fclose(cgroup_cpuset_fd);

    cgroup_cpuset_fd = fopen("/sys/fs/cgroup/cpuset/group1/cpuset.mems","w");
        if(cgroup_cpuset_fd==NULL){
        perror("[cpuset mem]Open fail fail");
        exit(1);
    }
    fprintf(cgroup_cpuset_fd,"0");
    fclose(cgroup_cpuset_fd);

    cgroup_procs_fd=fopen("/sys/fs/cgroup/cpuset/group1/cgroup.procs","w");
    if(cgroup_procs_fd==NULL){
        perror("[cpuset procs]Open file fail");
        exit(1);
    }
    fprintf(cgroup_procs_fd,"%d",pid);
    fclose(cgroup_procs_fd);
    return 0;
}