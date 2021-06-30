#include "cap.h"
#include <sys/prctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void list_capability(int flag){
    struct __user_cap_header_struct cap_header_data;
    cap_user_header_t cap_header = &cap_header_data;

    struct __user_cap_data_struct cap_data_data;
    cap_user_data_t cap_data = &cap_data_data;

    cap_header->pid = getpid();
    cap_header->version = _LINUX_CAPABILITY_VERSION_1;

    if (capget(cap_header, cap_data) < 0) {
        perror("Failed capget");
        exit(1);
    }
    if(flag==1) printf("[old]");
    else printf("[new]");
    printf("Cap data permitted: 0x%x, effective: 0x%x, inheritable:0x%xn\n", 
        cap_data->permitted, 
        cap_data->effective,
        cap_data->inheritable);
    return;
}

void init_capability(){

    cap_value_t cap_list[] = {
    CAP_SYS_CHROOT,
    CAP_SYS_ADMIN,
    CAP_SYS_TIME,
    CAP_SYS_BOOT,
    CAP_SYS_RAWIO,
    CAP_SYSLOG,
    CAP_DAC_READ_SEARCH,
    CAP_LINUX_IMMUTABLE,
    CAP_NET_BROADCAST,
    CAP_NET_ADMIN,
    CAP_IPC_LOCK,
    CAP_IPC_OWNER,
    CAP_SYS_MODULE,
    CAP_SYS_PTRACE,
    CAP_SYS_PACCT,
    CAP_SYS_NICE,
    CAP_SYS_RESOURCE,
    CAP_SYS_TTY_CONFIG,
    CAP_LEASE,
    CAP_AUDIT_CONTROL, 
    CAP_MAC_OVERRIDE,
    CAP_MAC_ADMIN,
    CAP_WAKE_ALARM,
    CAP_BLOCK_SUSPEND,
    };

    int res;
    for(int i=0;i<23;i++){
        res=prctl(PR_CAPBSET_DROP,cap_list[i]);
        if(res!=0){
            perror("prctl call fail\n");
            return;
        }
    }

    return;
}