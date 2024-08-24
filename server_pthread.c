#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

// 用于子线程传入参数的结构体
struct SockInfo{
    pthread_t tid;  // 线程号
    int cfd;  // 通信的文件描述符
    struct sockaddr_in address;  // 客户端地址结构体
};

// 子线程处理通信逻辑
void * working(void * arg) {
    struct SockInfo *addr = (struct SockInfo *)arg;

    // 获取客户端的ip地址和端口号
    char ip_addr[16];
    inet_ntop(AF_INET, &addr->address.sin_addr, ip_addr, sizeof(ip_addr));
    int port = ntohs(addr->address.sin_port);

    printf("IP(%s) client is connected\n", ip_addr);

    // 将客户端ip与端口号写入字符数组
    char write_buff[1024];
    sprintf(write_buff, "Your IP is %s; port is %d", ip_addr, port);

    
    // 循环将信息写回客户端，但是需要通过read判断客户端是否关闭，否则write时会报错
    int len;
    char recv_buff[1024];
    while(1){
        len = read(addr->cfd, recv_buff, sizeof(recv_buff));
        if (len == -1) {
            perror("Read error");
            exit(-1);
        } else if (len > 0) {  // 能正常读取到数据
            // printf("%s\n", recv_buff);
        } else if (len == 0) {
            printf("IP(%s) client is closed\n", ip_addr);
            break;
        }
        write(addr->cfd, write_buff, strlen(write_buff) + 1);
        sleep(2);
    }

    close(addr->cfd);   // 进程结束，关闭通信文件描述符
    return NULL;
}

int main(){
    // 创建监听socket的文件描述符
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    // 创建监听socket的IPV4 socket地址
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);  // INADDR_ANY取出主机中有效的任意IP地址
    address.sin_port = htons(8000);

    char buf[16];
    inet_ntop(AF_INET, &address.sin_addr, buf, sizeof(buf));
    printf("net is: %s:\n", buf);

    int flag = 1;
    // 设定端口允许被重复使用 (端口复用) 
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    // 绑定socket和地址
    int ret = bind(listenfd, (const struct sockaddr *)&address, sizeof(address));
    // 创建监听队列存放待处理的客户连接，设置监听的上限
    ret = listen(listenfd, 5);

    // 循环创建多线程的方式与客户端连接
    while (1)
    {
        struct sockaddr_in addr_info;
        socklen_t addr_size = sizeof(addr_info);
        int cfd = accept(listenfd, (struct sockaddr *)&addr_info, &addr_size);
 
        // 定义SockInfo类型结构体，并赋予初值
        struct SockInfo* sockinfo  = malloc(sizeof(struct SockInfo));
        sockinfo->cfd = cfd;
        memcpy(&sockinfo->address, &addr_info, addr_size);
        
        // 创建子线程用于通信
        pthread_create(&sockinfo->tid, NULL, working, (void*)sockinfo);
        pthread_detach(sockinfo->tid);    // 线程分离，系统回收
    }
    close(listenfd);
    return 0;
}
