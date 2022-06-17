#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
int main(){
    int sockfd;
    char buffer[1024];
    struct sockaddr_in server={0};
    int port = 30001, nbytes;
    char *serverip = "192.168.110.127";

    if((sockfd = socket(AF_INET , SOCK_STREAM , 0))==-1){
        perror("Socket fail : ");
        exit(1);
    }
    server.sin_family = AF_INET;

    if(inet_aton(serverip , &server.sin_addr)==0){
        printf("The server IP is not right !\n");
        exit(1);
    }
    server.sin_port = htons(port);
    if((connect(sockfd , (struct sockaddr *)(&server) , sizeof(struct sockaddr)))==-1){
        perror("Connect fail : ");
        exit(1);
    }
    nbytes = read(sockfd , buffer , 1024);
    if(nbytes <= 0){
        printf("Read error:%s\n",strerror(errno));
        exit(1);
    }
    close(sockfd);
    printf("Received %d bytes :%s\n",nbytes , buffer);
    return 0;
}
