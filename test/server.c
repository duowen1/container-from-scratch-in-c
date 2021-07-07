#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(){
    int sockfd,new_fd;
    int sin_size;
    int port=5050;
    struct sockaddr_in server={0};
    struct sockaddr_in client={0};

    sockfd=socket(AF_INET,SOCK_STREAM,0);

    if(sockfd==-1){
        perror("Socket error");
        exit(1);
    }


    server.sin_family=AF_INET;
    server.sin_addr.s_addr=htonl(INADDR_ANY);
    server.sin_port=htons(port);

    sin_size = sizeof(struct sockaddr);

    if(-1==bind(sockfd,(struct sockaddr *)(&server),sizeof(struct sockaddr))){
        perror("Bind error");
        exit(1);
    }

    if(-1==listen(sockfd,1)){
        perror("Listen error");
        exit(1);
    }

    
    new_fd=accept(sockfd,(struct sockaddr*)(&client),&sin_size);
    if(new_fd==-1){
        perror("Accept error");
        exit(1);
    }

    if(write(new_fd,"hello from the other side",strlen("hello from the other side"))==-1){
        perror("Write error");
        exit(-1);
    }

    close(new_fd);
    close(sockfd);

    return 0;
}