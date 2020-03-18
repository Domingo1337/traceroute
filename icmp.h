#include <inttypes.h>
#include <stdlib.h>

#define PACKETS_PER_TTL 3
#define TTL_RANGE 30

int is_valid_ipaddr(const char *ip_addr);

int receive_packet(int sockfd, char *sender_ip_str);

ssize_t send_echo_packet(int sockfd, uint16_t id, uint16_t seq, const char *ip_addr, int ttl);
