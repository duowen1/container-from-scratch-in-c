#include "cgroup.h"

void cgroup(pid_t pid, char * name){
    init_cpu_cgroup(pid, name);
    init_cpuset_cgroup(pid, name); 
    init_memory_cgroup(pid, name);
    init_freezer_cgroup(pid, name);
}

int init_cpu_cgroup(pid_t pid, char * name){
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

int init_cpuset_cgroup(pid_t pid, char * name){
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

int init_memory_cgroup(pid_t pid, char * name){
    FILE *cgroup_memorylimit_fd = NULL;
    FILE *cgroup_oomcontrol_fd = NULL;
    FILE *cgroup_swap_fd = NULL;
    FILE *cgroup_procs_fd = NULL;
    mkdir("/sys/fs/cgroup/memory/group1/",0755);

    cgroup_memorylimit_fd=fopen("/sys/fs/cgroup/memory/group1/memory.limit_in_bytes","w");
    if(cgroup_memorylimit_fd==NULL){
        perror("[cpuset]Open fail fail");
        exit(1);
    }
    fprintf(cgroup_memorylimit_fd,"%d",MEMORY_LIMIT);
    fclose(cgroup_memorylimit_fd);

    cgroup_oomcontrol_fd = fopen("/sys/fs/cgroup/memory/group1/memory.oom_control","w");
    if(cgroup_oomcontrol_fd==NULL){
        perror("[cpuset mem]Open fail fail");
        exit(1);
    }
    fprintf(cgroup_oomcontrol_fd,"0");
    fclose(cgroup_oomcontrol_fd);

    cgroup_swap_fd = fopen("/sys/fs/cgroup/memory/group1/memory.swappiness","w");
    fprintf(cgroup_swap_fd, "0");
    fclose(cgroup_swap_fd);

    cgroup_procs_fd=fopen("/sys/fs/cgroup/memory/group1/cgroup.procs","w");
    if(cgroup_procs_fd==NULL){
        perror("[cpuset procs]Open file fail");
        exit(1);
    }
    fprintf(cgroup_procs_fd,"%d",pid);
    fclose(cgroup_procs_fd);
    return 0;
}

int init_freezer_cgroup(pid_t pid, char * name){
    FILE *cgroup_procs_fd = NULL;
    mkdir("/sys/fs/cgroup/freezer/group1/",0755);

    cgroup_procs_fd = fopen("/sys/fs/cgroup/freezer/group1/cgroup.procs","w");
    if(cgroup_procs_fd==NULL){
        perror("[freezer]Open file fail.");
        exit(1);
    }

    fprintf(cgroup_procs_fd,"%d",pid);
    fclose(cgroup_procs_fd);

    return 0;
}

int freeze(char * name, int direction){
    FILE *cgroup_freezer_state = NULL;

    cgroup_freezer_state = fopen("/sys/fs/cgroup/freezer/group1/freezer.state","w");
    if(cgroup_freezer_state == NULL){
        perror("[freezer]Open freezer fail.");
        exit(1);
    }
    if(direction == FREEZE){
        fprintf(cgroup_freezer_state,"FROZEN");
    }else{
        fprintf(cgroup_freezer_state,"THAWED");
    }
    return 0;
}