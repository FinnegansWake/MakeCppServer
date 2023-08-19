#include <netinet/in.h>
#include <strings.h>
#include <sys/_endian.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in listen_addr;
    bzero(&listen_addr, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    listen_addr.sin_port = htons(12456);

    connect(sockfd, (sockaddr*)&listen_addr, sizeof(listen_addr));
    return 0;
}