#include <stdio.h>
#include "ece454rpc_types.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <string.h>
#include <stdarg.h>
#include <netdb.h> //hostent

unsigned char *serialize_int(unsigned char *buffer, int value);
const int port = 10069;

return_type make_remote_call(const char *servernameorip,
								const int serverportnumber,
								const char *procedure_name,
								const int nparams, ...) {
	/* create a udp socket and bind it */

	int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in socketaddress;
	memset((char *)&socketaddress, 0, sizeof(socketaddress));
	socketaddress.sin_family = AF_INET;
	socketaddress.sin_addr.s_addr = htonl(INADDR_ANY);
	socketaddress.sin_port = htons(port);
	bind(udp_socket, (struct sockaddr *)&socketaddress, sizeof(socketaddress));
	
	/* create the server address struct */
	struct sockaddr_in serveraddress;
	memset((char *)&serveraddress, 0, sizeof(serveraddress));
	serveraddress.sin_family = AF_INET;
	serveraddress.sin_port = htons(serverportnumber);
	
	struct hostent *serveripaddress;     
	serveripaddress = gethostbyname(servernameorip);
	
	/* put the host's address into the server address structure */
	memcpy((void *)&serveraddress.sin_addr, serveripaddress->h_addr_list[0], 
		serveripaddress->h_length);
	
	/* construct the array holding the function information
	   send a message from client to server */
		
	unsigned char buffer[512];
	memset(buffer, 0, sizeof(buffer));

	/* [size of func name|funcname|size of arg1|arg 1|size of arg 2|arg 2|...] */

	int functionnamesize = sizeof(procedure_name);
	unsigned char *tmp = serialize_int(buffer, functionnamesize);
  	
  	int i;
  	unsigned char *castedfunctionname = (unsigned char*)procedure_name;
  	for (i = 0; i < functionnamesize; i++) {
  		tmp[i] = castedfunctionname[i];
  	}
  	
  	tmp = tmp + functionnamesize;
  	tmp = serialize_int(tmp, nparams);

	va_list va;
    va_start(va, nparams);
    for (i = 0; i < nparams; i++) {
    	/* put size of arg in buffer */
    	int argsize = va_arg(va, int);
		tmp = serialize_int(tmp, argsize);
    	
    	/* put the arg in buffer */
    	void *arg = va_arg(va, void *);
    	unsigned char *castedarg = (unsigned char*)arg;
		printf("arg being passed: %d \n", *(int*)castedarg);
    	int j;
		for (j = 0; j < argsize; j++) {
			tmp[j] = castedarg[j];
		}
		printf("arg sending is: %d \n", *(int*)tmp);

		tmp = tmp + argsize;
    }
    va_end(va);
	
	sendto(udp_socket, buffer, sizeof(buffer), 0, 
		(struct sockaddr *)&serveraddress, sizeof(serveraddress));
	
	/* listen for return_val of rpc */
	struct sockaddr_in remoteaddress;     
    socklen_t addrlen = sizeof(remoteaddress);      
	unsigned char recvbuf[512];
	for (;;) {
		int recvlen;
		recvlen = recvfrom(udp_socket, (void *)recvbuf, sizeof(recvbuf), 
			0, (struct sockaddr *)&remoteaddress, &addrlen);
		if (recvlen > 0) {
			int returnsize = *(int*)recvbuf;
			int k;
			unsigned char *returnvalbuf = recvbuf + 4;
			unsigned char returnval[returnsize];

			for (k = 0; k < returnsize; k++) {
				returnval[k] = returnvalbuf[k];
			}
			
			return_type rt;
			memset((unsigned char *)&rt, 0, sizeof(rt));
			rt.return_size = returnsize;
			rt.return_val = returnval;
			
			return rt;
		}
	}
}

unsigned char *serialize_int(unsigned char *buffer, int value) {
	/* Write little-endian int value into buffer */
	buffer[0] = value >> 0;
	buffer[1] = value >> 8;
	buffer[2] = value >> 16;
	buffer[3] = value >> 24;
	return buffer + 4;
}
int main()
{
    int a =27, b = 91;
    return_type ans = make_remote_call("ecelinux3.uwaterloo.ca",
	                               10004, "addtwo", 2,
	                               sizeof(int), (void *)(&a),
	                               sizeof(int), (void *)(&b));
    int i = *(int *)(ans.return_val);
    printf("client, got result: %d\n", i);

    return 0;
}
