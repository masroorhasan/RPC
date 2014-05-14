#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <errno.h>

#define	PORT_RANGE_LO	10000
#define PORT_RANGE_HI	10100

/* 
 * mybind() -- a wrapper to bind that tries to bind() to a port in the
 * range PORT_RANGE_LO - PORT_RANGE_HI, inclusive.
 *
 * Parameters:
 *
 * sockfd -- the socket descriptor to which to bind
 *
 * addr -- a pointer to struct sockaddr_in. mybind() works for AF_INET sockets only.
 * Note that addr is and in-out parameter. That is, addr->sin_family and
 * addr->sin_addr are assumed to have been initialized correctly before the call.
 * Also, addr->sin_port must be 0, or the call returns with an error. Up on return,
 * addr->sin_port contains, in network byte order, the port to which the call bound
 * sockfd.
 *
 * returns int -- negative return means an error occurred, else the call succeeded.
 */
int mybind(int sockfd, struct sockaddr_in *addr) {
    if(sockfd < 1) {
	fprintf(stderr, "mybind(): sockfd has invalid value %d\n", sockfd);
	return -1;
    }

    if(addr == NULL) {
	fprintf(stderr, "mybind(): addr is NULL\n");
	return -1;
    }

    if(addr->sin_port != 0) {
	fprintf(stderr, "mybind(): addr->sin_port is non-zero. Perhaps you want bind() instead?\n");
	return -1;
    }

    unsigned short p;
    for(p = PORT_RANGE_LO; p <= PORT_RANGE_HI; p++) {
	addr->sin_port = htons(p);
	int b = bind(sockfd, (const struct sockaddr *)addr, sizeof(struct sockaddr_in));
	if(b < 0) {
	    continue;
	}
	else {
	    break;
	}
    }

    if(p > PORT_RANGE_HI) {
	fprintf(stderr, "mybind(): all bind() attempts failed. No port available...?\n");
	return -1;
    }

    /* Note: upon successful return, addr->sin_port contains, in network byte order, the
     * port to which we successfully bound. */
    return 0;
}
