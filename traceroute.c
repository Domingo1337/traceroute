#include "icmp.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define TTL_RANGE 30

int main(int argc, char *argv[]) {
    struct sockaddr_in recipient;
    bzero(&recipient, sizeof(recipient));
    recipient.sin_family = AF_INET;

    // read recipient's ip address from program's first argument
    if (argc < 2 || inet_pton(AF_INET, argv[1], &recipient.sin_addr) != 1) {
        fprintf(stderr, "Wrong arguments.\nUsage: [sudo] ./traceroute <ip_addr> \n");
        return EXIT_FAILURE;
    }

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        fprintf(stderr, "socket error: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    uint16_t pid = getpid();
    struct timeval time_now;
    for (int ttl = 1; ttl <= TTL_RANGE; ttl++) {
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
    fprintf(stderr, "Couldn't reach target in %d hops, aborting.\n", TTL_RANGE);
    return EXIT_FAILURE;
}
