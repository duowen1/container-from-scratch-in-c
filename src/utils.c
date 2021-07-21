#include "utils.h"

int checkroot(){
    if(getuid()){
        printf("run me as root");
        exit(1);
    }
}

char generate_random_char(){
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

