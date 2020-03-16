#include "icmp.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <string.h>


int is_valid_ipaddr(const char *ip_addr){
    struct sockaddr_in temp;
    return inet_pton(AF_INET, ip_addr, &temp.sin_addr) != 0;
}

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

static u_int16_t compute_icmp_checksum(const void *buff, size_t length) {
    u_int32_t sum;
    const u_int16_t *ptr = buff;
    assert(length % 2 == 0);
    for (sum = 0; length > 0; length -= 2)
        sum += *ptr++;
    sum = (sum >> 16) + (sum & 0xffff);
    return (u_int16_t)(~(sum + (sum >> 16)));
}

ssize_t send_echo_packet(int sockfd, uint16_t id, uint16_t seq, const char* ip_addr) {
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

    int ttl = 42;
    setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));

    ssize_t bytes_sent = sendto(sockfd, &header, sizeof(header), 0, (struct sockaddr *)&recipient, sizeof(recipient));

    return bytes_sent;
}