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