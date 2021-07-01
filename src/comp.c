#include "comp.h"
#include <stdio.h>

int init_comp(){
    int syscall[] = {
    SCMP_SYS(acct),
    SCMP_SYS(add_key),
    SCMP_SYS(bpf),
    SCMP_SYS(clock_adjtime),
    SCMP_SYS(clock_settime),
    //SCMP_SYS(clone),
    SCMP_SYS(create_module),
    SCMP_SYS(delete_module),
    SCMP_SYS(finit_module),
    SCMP_SYS(get_kernel_syms),
    SCMP_SYS(get_mempolicy),
    SCMP_SYS(init_module),
    SCMP_SYS(ioperm),
    SCMP_SYS(iopl),
    SCMP_SYS(kcmp),
    SCMP_SYS(kexec_file_load),
    SCMP_SYS(kexec_load),
    SCMP_SYS(keyctl),
    SCMP_SYS(lookup_dcookie),
    SCMP_SYS(mbind),
    SCMP_SYS(mount),
    SCMP_SYS(move_pages),
    SCMP_SYS(name_to_handle_at),
    SCMP_SYS(nfsservctl),
    SCMP_SYS(open_by_handle_at),
    SCMP_SYS(perf_event_open),
    SCMP_SYS(personality),
    SCMP_SYS(pivot_root),
    SCMP_SYS(process_vm_readv),
    SCMP_SYS(process_vm_writev),
    SCMP_SYS(ptrace),
    SCMP_SYS(query_module),
    SCMP_SYS(quotactl),
    SCMP_SYS(reboot),
    SCMP_SYS(request_key),
    SCMP_SYS(set_mempolicy),
    SCMP_SYS(setns),
    SCMP_SYS(settimeofday),
    SCMP_SYS(stime),
    SCMP_SYS(swapon),
    SCMP_SYS(swapoff),
    SCMP_SYS(sysfs),
    SCMP_SYS(_sysctl),
    SCMP_SYS(umount),
    SCMP_SYS(umount2),
    SCMP_SYS(unshare),
    SCMP_SYS(uselib),
    SCMP_SYS(userfaultfd),
    SCMP_SYS(ustat),
    SCMP_SYS(vm86),
    SCMP_SYS(vm86old)};
    
    scmp_filter_ctx scmp=seccomp_init(SCMP_ACT_ALLOW);

    if(!scmp){
        perror("failed to initialize libseccomp\n");
        return 1;
    }

    int lenth = sizeof(syscall) / sizeof(int);

    for(int i=0; i<lenth; i++){
        if(seccomp_rule_add(scmp,SCMP_ACT_KILL,syscall[i],0)<0){
            perror("seccomp_rule_add_fail");
            return 1;
        }
    }

    if(seccomp_load(scmp)!=0){
        perror("failed to load the filter in the kernnel\n");
        return 1;
    }
    return 0;
}