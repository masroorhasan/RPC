#include <stdio.h>
#include "ece454rpc_types.h"
#include "mybind.c"

#define BUFSIZE 2048

// Socket API
#include <sys/types.h>
#include <sys/socket.h>

// Struct sockaddr_in 
#include <netinet/in.h>
#include <netdb.h>
#include <inttypes.h>

// memset
#include <string.h>

int ret_int;
return_type r;

void paddr(unsigned char *a) {
	printf("%d.%d.%d.%d\n", a[0], a[1], a[2], a[3]);
}

return_type add(const int nparams, arg_type* a)
{
    if(nparams != 2) {
	/* Error! */
	r.return_val = NULL;
	r.return_size = 0;
	return r;
    }

    if(a->arg_size != sizeof(int) ||
       a->next->arg_size != sizeof(int)) {
	/* Error! */
	r.return_val = NULL;
	r.return_size = 0;
	return r;
    }

    int i = *(int *)(a->arg_val);
    int j = *(int *)(a->next->arg_val);

    ret_int = i+j;
    r.return_val = (void *)(&ret_int);
    r.return_size = sizeof(int);

    return r;
}


/* register_procedure() -- invoked by the app programmer's server code
 * to register a procedure with this server_stub. Note that more than
 * one procedure can be registered */
extern bool register_procedure(const char *procedure_name, 
                    const int nparams, fp_type fnpointer)
{
    //define array (db) to store fp's
    //put procedure_name as fp in an array of fp's
    return false;
}

/* launch_server() -- used by the app programmer's server code to indicate that
 * it wants start receiving rpc invocations for functions that it registered
 * with the server stub. */
void launch_server() {        
    struct sockaddr_in myaddr;      /* our address */
        struct sockaddr_in remaddr;     /* remote address */
        socklen_t addrlen = sizeof(remaddr);            /* length of addresses */
        int recvlen;                    /* # bytes received */
        int fd;                         /* our socket */
        unsigned char buf[BUFSIZE];     /* receive buffer */

        /* create a UDP socket */

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket\n");
        return;
    }

    /* bind the socket to any valid IP address and a specific port */
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //myaddr.sin_port = htons(PORT);


    if(mybind(fd, (struct sockaddr_in*)&myaddr) < 0 ) {
        perror("Server couldn't bind port to socket.");
        return; 
    }

    printf("Server binded socket to port: %i \n", ntohs(myaddr.sin_port));


    /* now loop, receiving data and printing what we received */
    for (;;) {
        printf("waiting on port %d\n", ntohs(myaddr.sin_port));
        recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
        printf("received %d bytes\n", recvlen);
        if (recvlen > 0) {
            buf[recvlen] = 0;
            printf("received message: \"%s\"\n", buf);
        }
    }
        /* never exits */
}

int main() {
    register_procedure("addtwo", 2, add);

    launch_server();

    /* should never get here, because
       launch_server(); runs forever. */

    return 0;
}
