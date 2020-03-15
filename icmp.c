#include "icmp.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_as_bytes(unsigned char *buff, size_t length) {
    for (size_t i = 0; i < length; i++, buff++)
        printf("%.2x ", *buff);
}

int receive_packet(int sockfd) {
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    u_int8_t buffer[IP_MAXPACKET];

    ssize_t packet_len = recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr *)&sender, &sender_len);
    if (packet_len < 0) {
        fprintf(stderr, "recvfrom error: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    struct iphdr *ip_header = (struct iphdr *)buffer;
    char sender_ip_str[20];
    inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_str, sizeof(sender_ip_str));
    printf("Received IP packet with ICMP content from: %s\n", sender_ip_str);

    size_t ip_header_len = 4 * ip_header->ihl;

    printf("IP header: ");
    print_as_bytes(buffer, ip_header_len);
    printf("\n");

    printf("IP data:   ");
    print_as_bytes(buffer + ip_header_len, packet_len - ip_header_len);
    printf("\n\n");

    return EXIT_SUCCESS;
}
