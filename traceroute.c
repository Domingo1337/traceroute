#include "icmp.h"

#include <errno.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2 || !is_valid_ipaddr(argv[1])) {
        fprintf(stderr, "Wrong arguments.\nUsage: sudo ./traceroute <ip_addr> \n");
        return EXIT_FAILURE;
    }

    printf("Let's trace some routes!\n");

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if (sockfd < 0 || setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        fprintf(stderr, "socket error: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    uint16_t pid = getpid();
    struct timeval time_now;

    for (int ttl = 1; ttl <= TTL_RANGE; ttl++)
        if (send_echo_packets(sockfd, pid, ttl, ttl, argv[1]) > 0) {
            gettimeofday(&time_now, NULL);
            printf("%u. \t", ttl);
            if (receive_packets(sockfd, time_now) == 1)
                return EXIT_SUCCESS;
        }
}
