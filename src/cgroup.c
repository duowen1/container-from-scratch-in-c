#include "cgroup.h"

void cgroup(pid_t pid){
    init_cpu_cgroup(pid);
    init_cpuset_cgroup(pid); 
    init_memory_cgroup(pid);
    init_cgroup_v2(pid);
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

int init_memory_cgroup(pid_t pid){
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

int init_cgroup_v2(pid_t pid){

    mkdir("/sys/fs/cgroup/unified/group1",0755);//use cgroup v2 to limit the bandwith of io
    FILE * cgroup_subtree_fd;
    FILE * cgroup_procs_fd;
    FILE * cgroup_iomax_fd;

    cgroup_subtree_fd = fopen("/sys/fs/cgroup/unified/cgroup.subtree_control","w");
    if(cgroup_subtree_fd == NULL){
        perror("[cgroup v2]Open subtree fail.");
        exit(1);
    }
    fprintf(cgroup_subtree_fd,"+io +memory");
    fclose(cgroup_subtree_fd);

    cgroup_procs_fd = fopen("/sys/fs/cgroup/unified/group1/cgroup.procs","w");
    if(cgroup_procs_fd == NULL){
        perror("[cgroup v2]Open cgroup.procs fail");
        exit(1);
    }
    fprintf(cgroup_procs_fd,"%d",pid);
    fclose(cgroup_procs_fd);

    cgroup_iomax_fd = fopen("/sys/fs/cgroup/unified/group1/io.max");
    if(cgroup_iomax_fd == NULL){
        perror("[cgroup v2]Open io.max fail");
        exit(1);
    }
    fprintf(cgroup_iomax_fd,"%d:%d wbps=%d",8,5,10485760);
    fclose(cgroup_iomax_fd);

    return 0;


}