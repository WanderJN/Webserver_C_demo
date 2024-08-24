#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


int main(){
    // 创建socket的文件描述符
    int cfd = socket(PF_INET, SOCK_STREAM, 0);
    // 创建监听socket的IPV4 socket地址
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &address.sin_addr.s_addr);
    address.sin_port = htons(8000);

    connect(cfd, (struct sockaddr *)&address, sizeof(address));
    char recv_buff[1024];
    int len;
    while (1)
    {
        write(cfd, "I'm running", strlen("I'm running") + 1);    // 向服务器写空值
        len = read(cfd, recv_buff, sizeof(recv_buff));
        if (len == -1) {
            perror("Read error");
            exit(-1);
        } else if (len > 0) {
            printf(":%s\n", recv_buff);
        } else if (len == 0) {
            break;
        }
    }

    close(cfd);
    return 0;
}
