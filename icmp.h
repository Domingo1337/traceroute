#include <inttypes.h>
#include <stdlib.h>
#include <sys/time.h>

#define PACKETS_PER_TTL 3
#define TTL_RANGE 30

int is_valid_ipaddr(const char *ip_addr);

// return 1 on host reached, 0 otherwise
int receive_packets(int sockfd, uint16_t id, uint16_t seq, struct timeval time_send);

ssize_t send_echo_packets(int sockfd, uint16_t id, uint16_t seq, int ttl, const char *ip_addr);
