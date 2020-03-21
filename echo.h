#include <inttypes.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <sys/time.h>

#define PACKETS_PER_TTL 3
#define SEC_FOR_ANSWER 1

/*
 * Receive ICMP packets from socket sockfd, accepting only answers to echo request with matching id and seq.
 * time_until is expected to be current time + SEC_FOR_ANSWER seconds.
 * Function returns either after accepting PACKETS_PER_TTL packets or after current time reaches time_until.
 * Return value: 1 if received correct ICMP packet of type ECHO_REPLY, -1 on error, 0 otherwise.
 */
int receive_packets(int sockfd, uint16_t id, uint16_t seq, struct timeval time_until);

/*
 * Send PACKETS_PER_TTL ICMP echo request packets to recipient with specified id, seq, and ttl using socket sockfd.
 * Return value: -1 on error, 0 otherwise.
 */
int send_echo_packets(int sockfd, uint16_t id, uint16_t seq, int ttl, struct sockaddr *recipient);
