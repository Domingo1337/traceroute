#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "echo.h"

// program sends ICMP echo reply packets with increasing ttl in range [1..TTL_RANGE]
#define TTL_RANGE 30

int main(int argc, char *argv[]) {
    // store target recipient data
    struct sockaddr_in recipient;
    bzero(&recipient, sizeof(recipient));
    recipient.sin_family = AF_INET;

    // read recipient's ip address from program's first argument
    if (argc < 2 || inet_pton(AF_INET, argv[1], &recipient.sin_addr) != 1) {
        fprintf(stderr, "Wrong arguments.\nUsage: [sudo] ./traceroute <ip_addr> \n");
        return EXIT_FAILURE;
    }

    // prepare socket
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        fprintf(stderr, "socket error: %s\nMake sure you have root privileges, needed to create raw sockets.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    struct timeval time_now;
    
    // use program's pid and ttl to identify sent packets
    uint16_t pid = getpid();
    for (uint16_t ttl = 1; ttl <= TTL_RANGE; ttl++) {
        if (send_echo_packets(sockfd, pid, ttl, ttl, (struct sockaddr *)&recipient) == 0) {
            assert(gettimeofday(&time_now, NULL) == 0);
            time_now.tv_sec += SEC_FOR_ANSWER;
            printf("%2u. ", ttl);
            fflush(stdout);

            switch (receive_packets(sockfd, pid, ttl, time_now)) {
            case 1:
                return EXIT_SUCCESS;
            case -1:
                return EXIT_FAILURE;
            default:
                continue;
            }
        } else {
            return EXIT_FAILURE;
        }
    }
    fprintf(stderr, "Couldn't reach target %s in %d hops, aborting.\n", argv[1], TTL_RANGE);
    return EXIT_FAILURE;
}
