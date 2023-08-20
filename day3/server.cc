#include <netinet/in.h>
#include <strings.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "util.h"
#include <errno.h>

/* 
    The code snippet you provided is a C function named `setnonblocking` that takes an integer file descriptor `fd` as an argument and sets the non-blocking mode for that file descriptor using the `fcntl` function. Let's break down what each part of the code does:

1. `fcntl(fd, F_GETFL)`: This part retrieves the file status flags of the given file descriptor `fd`. It is used to get the current flags associated with the file descriptor.

2. `fcntl(fd, F_GETFL) | O_NONBLOCK`: This part uses a bitwise OR operation (`|`) to combine the current file status flags with the `O_NONBLOCK` flag. The `O_NONBLOCK` flag is used to enable non-blocking mode on the file descriptor.

3. `fcntl(fd, F_SETFL, ... )`: This part is responsible for setting the modified file status flags back to the file descriptor. It uses the `F_SETFL` command to set the new flags.

So, the overall effect of the code is to modify the file descriptor's status flags to include the `O_NONBLOCK` flag, thereby making I/O operations on that file descriptor non-blocking.

Here's a brief explanation of what non-blocking mode means:

- In a non-blocking file descriptor, I/O operations (like reading or writing) will not wait for data to be available or space to be available for writing. If there's no data available for reading or no space available for writing, the operation will return immediately with a special error code or condition that indicates that the operation would have blocked.

- This is in contrast to blocking I/O, where operations would wait until they can be completed. Non-blocking I/O is often used in situations where you don't want a single operation to block the entire program's execution.

Keep in mind that proper error handling should be added to this code to handle cases where the `fcntl` calls fail.
*/
void setnonblocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

// Use epoll edge trigger mode
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

    int epfd = epoll_create1(0);
    errif(epfd == -1, "epoll create error");

    struct epoll_event events[MAX_EVENTS], ev;
    bzero(&events, sizeof(events));
    bzero(&ev, sizeof(ev));
    ev.data.fd = sockfd;
    ev.events = EPOLLIN | EPOLLET;
    setnonblocking(sockfd);
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    while (true) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        errif(nfds == -1, "epoll_wait error");
        for (int i = 0; i < nfds; ++i) {
            // sockfd用于accept，代表新的客户端连接出现
            if (events[i].data.fd == sockfd) {
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                bzero(&client_addr, client_addr_len);

                int client_sockfd = accept(sockfd, (sockaddr*)&client_addr, &client_addr_len);
                errif(client_sockfd == -1, "socket accpet error");
                printf("new client fd %d! IP: %s Port: %d\n", client_sockfd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                ev.data.fd = client_sockfd;
                ev.events = EPOLLIN | EPOLLET;
                setnonblocking(client_sockfd);
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_sockfd, &ev);
            } else if (events[i].events & EPOLLIN) {
                // 边缘触发，一次性把buffer都读完
                char buf[BUFFER_SIZE];
                while (true) {
                    bzero(&buf, sizeof(buf));
                    auto client_accessfd = events[i].data.fd;
                    auto bytes_read = read(client_accessfd, buf, sizeof(buf));
                    if (bytes_read > 0) {
                        printf("message from client fd %d: %s\n", client_accessfd, buf);
                        // echo逻辑
                        write(client_accessfd, buf, bytes_read);
                    } else if (bytes_read == -1 && errno == EINTR) {
                        // 慢系统调用捕获信号
                        printf("Catch signal, continue");
                        continue;
                    } else if (bytes_read == -1 && ((errno == EAGAIN || errno == EWOULDBLOCK))) {
                        printf("EAGAIN Or EWOULDBLOCK: %d\n", errno);
                        break;
                    } else if (bytes_read == 0) {
                        printf("EOF, client fd: %d disconnected\n", client_accessfd);
                        close(client_accessfd);
                        break;
                    }
                } 
            } else {
                printf("To Be Continued...\n");
            }
        }
    }
    close(sockfd);
    return 0;
}