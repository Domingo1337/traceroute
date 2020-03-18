#include "icmp.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <string.h>

int is_valid_ipaddr(const char *ip_addr) {
    struct sockaddr_in temp;
    return inet_pton(AF_INET, ip_addr, &temp.sin_addr) != 0;
}

int receive_packet(int sockfd, char *sender_ip_str) {
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    u_int8_t buffer[IP_MAXPACKET];

    for (int i = 0; i < PACKETS_PER_TTL; i++) {
        ssize_t packet_len = recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr *)&sender, &sender_len);
        if (packet_len < 0) {
            fprintf(stderr, "recvfrom error: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }

        struct iphdr *ip_header = (struct iphdr *)buffer;
        inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_str, 20U);

        struct icmphdr *icmp_header = (struct icmphdr *)(buffer + 4 * ip_header->ihl);

        const char *type = "ECHOREPLY    ";
        if (icmp_header->type == ICMP_TIME_EXCEEDED) {
            type = "TIME_EXCEEDED";
            ip_header = (struct iphdr *)(icmp_header + 1);
            icmp_header = (struct icmphdr *)((uint8_t *)ip_header + 4 * ip_header->ihl);
        } else if (icmp_header->type != ICMP_ECHOREPLY) {
            printf("THAT'S WEIRD, PACKET CODE IS %u", icmp_header->type);
        }

        printf("%s\t id:%u \t ttl:%u  \t%s\n", type, icmp_header->un.echo.id, icmp_header->un.echo.sequence,
               sender_ip_str);
    }

    return EXIT_SUCCESS;
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

ssize_t send_echo_packet(int sockfd, uint16_t id, uint16_t seq, const char *ip_addr, int ttl) {
    struct icmphdr header;
    header.type = ICMP_ECHO;
    header.code = 0;
    header.un.echo.id = id;
    header.un.echo.sequence = seq;
    header.checksum = 0;
    header.checksum = compute_icmp_checksum(&header, sizeof(header));

    struct sockaddr_in recipient;
    bzero(&recipient, sizeof(recipient));
    recipient.sin_family = AF_INET;
    inet_pton(AF_INET, ip_addr, &recipient.sin_addr);

    setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));

    ssize_t bytes_sent = 0;
    for (int i = 0; i < PACKETS_PER_TTL; i++)
        bytes_sent += sendto(sockfd, &header, sizeof(header), 0, (struct sockaddr *)&recipient, sizeof(recipient));

    return bytes_sent;
}