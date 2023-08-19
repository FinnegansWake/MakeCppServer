#include <cstdio>
#include <netinet/in.h>
#include <strings.h>
#include <sys/_endian.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>

#include "util.h"

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in listen_addr;
    bzero(&listen_addr, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    listen_addr.sin_port = htons(12456);

    errif(connect(sockfd, (sockaddr*)&listen_addr, sizeof(listen_addr)) == -1, "socket connect error");

    while (true) {
        char buf[BUFFER_SIZE];
        bzero(buf, sizeof(buf));
        scanf("%s", buf);
        auto write_bytes = write(sockfd, buf, sizeof(buf));
        if (write_bytes == -1) {
            printf("socket already disconnected, can't write any more\n");
            break;
        }
        bzero(buf, sizeof(buf));
        auto read_bytes = read(sockfd, buf, sizeof(buf));
        if (read_bytes > 0) {
            printf("message from server: %s\n", buf);
        } else if (read_bytes == 0) {
            printf("server fd %d disconnected", sockfd);
            break;
        } else if (write_bytes == -1) {
            close(sockfd);
            errif(true, "socket read error");
        }
    }
    close(sockfd);
    return 0;
}