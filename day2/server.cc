#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <cstdio>
#include <unistd.h>

#include "util.h"
int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(12456);

    errif(bind(sockfd, (sockaddr* )&serv_addr, sizeof(serv_addr)) == -1, "socket bind error");
    
    errif(listen(sockfd, SOMAXCONN) == -1, "socket listen error");

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    bzero(&client_addr, sizeof(client_addr));

    int client_sockfd = accept(sockfd, (sockaddr*)&client_addr, &client_addr_len);
    errif(client_sockfd == -1, "socket accept");

    printf("new client fd %d! IP: %s Port: %d\n", client_sockfd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    while (true) {
        char buf[BUFFER_SIZE];
        bzero(buf, sizeof(buf));
        auto read_bytes = read(client_sockfd, buf, sizeof(buf));
        if (read_bytes > 0) {
            printf("message from client fd %d: %s\n", client_sockfd, buf);
            write(client_sockfd, buf, sizeof(buf));
        } else if (read_bytes == 0) {
            printf("client fd %d disconnected", client_sockfd);
            close(client_sockfd);
            break;
        } else if (read_bytes == -1) {
            close(client_sockfd);
            errif(true, "socket read error");
        }
    }
    close(sockfd);
    return 0;
}