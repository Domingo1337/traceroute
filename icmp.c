#include "icmp.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <string.h>

int receive_packets(int sockfd, uint16_t id, uint16_t seq, struct timeval time_until) {
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    u_int8_t buffer[IP_MAXPACKET];
    char sender_ip_str[PACKETS_PER_TTL][16];
    float time_sum = 0.f;
    struct timeval time_left;
    struct timeval time_now;

    int i = 0;
    int host_reached = 0;

    while (i < PACKETS_PER_TTL && gettimeofday(&time_now, NULL) == 0 && timercmp(&time_now, &time_until, <) == 1) {
        timersub(&time_until, &time_now, &time_left);
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &time_left, sizeof(time_left)) < 0) {
            fprintf(stderr, "setsockopt error: %s\n", strerror(errno));
            return -1;
        }

        if (recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr *)&sender, &sender_len) == -1) {
            if (errno == EWOULDBLOCK)
                break;
            fprintf(stderr, "recvfrom error: %s\n", strerror(errno));
            return -1;
        }
        assert(gettimeofday(&time_now, NULL) == 0);

        struct iphdr *ip_header = (struct iphdr *)buffer;
        inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_str[i], 20U);

        struct icmphdr *icmp_header = (struct icmphdr *)(buffer + 4 * ip_header->ihl);

        if (icmp_header->type == ICMP_TIME_EXCEEDED) {
            ip_header = (struct iphdr *)((uint8_t *)icmp_header + 8);
            icmp_header = (struct icmphdr *)((uint8_t *)ip_header + 4 * ip_header->ihl);
        } else if (icmp_header->type != ICMP_ECHOREPLY) {
            continue;
        }

        if (icmp_header->un.echo.id != id || icmp_header->un.echo.sequence != seq)
            continue;

        time_sum += (time_now.tv_sec - time_until.tv_sec + SEC_FOR_ANSWER) * 1000 +
                    (time_now.tv_usec - time_until.tv_usec) / 1000.f;

        if (icmp_header->type == ICMP_ECHOREPLY) {
            host_reached = 1;
        }

        int should_print_ip = 1;
        for (int j = 0; j < i; j++)
            should_print_ip = should_print_ip && strncmp(sender_ip_str[i], sender_ip_str[j], 16);
        if (should_print_ip)
            printf("%-16s", sender_ip_str[i]);

        i++;
    }
    if (i == PACKETS_PER_TTL)
        printf("%.0fms\n", time_sum / PACKETS_PER_TTL);
    else
        printf(i == 0 ? "*\n" : "???\n");
    return host_reached;
}

static u_int16_t compute_icmp_checksum(const void *buff, size_t length) {
    u_int32_t sum;
    const u_int16_t *ptr = buff;
    assert(length % 2 == 0);
    for (sum = 0; length > 0; length -= 2)
        sum += *ptr++;
    sum = (sum >> 16) + (sum & 0xffff);
    return (u_int16_t)(~(sum + (sum >> 16)));
}

int send_echo_packets(int sockfd, uint16_t id, uint16_t seq, int ttl, struct sockaddr *recipient) {
    struct icmphdr header;
    header.type = ICMP_ECHO;
    header.code = 0;
    header.un.echo.id = id;
    header.un.echo.sequence = seq;
    header.checksum = 0;
    header.checksum = compute_icmp_checksum(&header, sizeof(header));

    if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int)) < 0) {
        fprintf(stderr, "setsockopt error: %s\n", strerror(errno));
        return -1;
    }

    for (int i = 0; i < PACKETS_PER_TTL; i++) {
        if (sendto(sockfd, &header, sizeof(header), 0, recipient, sizeof(struct sockaddr)) < 0) {
            fprintf(stderr, "sendto error: %s\n", strerror(errno));
            return -1;
        }
    }

    return 0;
}
