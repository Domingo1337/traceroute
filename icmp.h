#include <inttypes.h>
#include <stdlib.h>

int is_valid_ipaddr(const char *ip_addr);

int receive_packet(int sockfd);

ssize_t send_echo_packet(int sockfd, uint16_t id, uint16_t seq, const char *ip_addr);
