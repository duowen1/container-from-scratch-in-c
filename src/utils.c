#include "utils.h"

static char generate_random_char();

void checkroot(){
    if(getuid()){
        printf("run me as root");
        exit(1);
    }
}

static char generate_random_char(){
    int flag = rand() % 16;
    if(flag <= 9){
        return ('0'+flag);
    }else{
        return ('a'+flag-10);
    }
}

char * generate_random_string(int len){
    char * name;
    name = malloc(sizeof(char) * (len + 1));
    memset(name,0,sizeof(name));
    srand((unsigned)time(NULL));
    for(int i = 0; i < len; i++){
        *(name+i) = generate_random_char();
    }
    return name;
}

//"return 0" means matched
int isstartwith(char* cmd, char *container_name){
    int len = strlen(cmd);
    return strncmp(cmd, container_name, len);
}