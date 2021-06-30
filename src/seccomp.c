#include <seccomp.h>
#include <stdio.h>

extern void init_seccomp();

void init_seccomp(){
    scmp_filter_ctx scmp=seccomp_init(SCMP_ACT_ALLOW);

    if(!scmp){
        perror("failed to initialize libseccomp\n");
        return;
    }

    if(seccomp_rule_add(scmp,SCMP_ACT_KILL,SCMP_SYS(unshare),0)<0){
        perror("seccomp_rule_add_fail");
        return;

    }

    if(seccomp_rule_add(scmp,SCMP_ACT_KILL,SCMP_SYS(setns),0)<0){
        perror("seccomp_rule_add_fail");
        return;
    }


    if(seccomp_load(scmp)!=0){
        perror("failed to load the filter in the kernnel\n");
        return;
    }

}