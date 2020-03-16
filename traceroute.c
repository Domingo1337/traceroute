#include "icmp.h"

#include <errno.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <string.h>

#include <sys/time.h>

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

    char sender_ip_str[20];
    for (int ttl = 1; ttl <= 30; ttl++)
        if (send_echo_packet(sockfd, 1, 1, argv[1], ttl)) {
            receive_packet(sockfd, sender_ip_str);
            if (strcmp(argv[1], sender_ip_str) == 0)
                return EXIT_SUCCESS;
        }
}
