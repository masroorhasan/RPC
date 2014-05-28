/**
 * Sources:
 * http://www.cs.rutgers.edu/~pxk/417/notes/sockets/udp.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mybind.c"
#include "ece454rpc_types.h"

// Socket
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>

// memcpy
#include <string.h>

// variable arguments
#include <stdarg.h>

#define BUFSIZE 512

typedef struct {
    char *proc_name;
    int num_params;
    arg_type arguments;
} proc_dec_type;

/**
 * Prints an IP address in dotted decimal notation.
 */
void paddr(unsigned char *a) {
    printf("%d.%d.%d.%d\n", a[0], a[1], a[2], a[3]);
}

/**
 * Creates a socket.
 */
int createSocket(const int domain, const int type, const int protocol) {

    int socketDescriptor = socket(domain, type, protocol);

    if (socketDescriptor == -1) {
        perror("Failed to create socket.");
        exit(0);
    }

    printf("Created socket.\n");
    return socketDescriptor;
}

/**
 * Assigns socket an IP address and random port.
 */
void bindSocket(int *socket) {

    struct sockaddr_in myAddress;

    // Let OS pick what IP address is assigned
    myAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    myAddress.sin_family = AF_INET;
    myAddress.sin_port = 0;

    // Assign port
    if(mybind(*socket, (struct sockaddr_in*)&myAddress) < 0 ) {
        perror("Could't bind port to socket.");
        exit(0);
    }

    printf("Binded socket to port: %i \n", ntohs(myAddress.sin_port));
}

/**
 * Fetches host details for a given IP address.
 */
struct hostent* getHostDetails(const char *ipAddress) {

    struct hostent *host;
    host = gethostbyname(ipAddress);
    if (!host) {
        fprintf(stderr, "Could not obtain IP address for %s\n", ipAddress);
    }

    printf("The IP address of %s is: ", ipAddress);
    paddr((unsigned char*) host->h_addr_list[0]);

    return host;
}

int serializeData(const char *procedure_name, int nparams, va_list valist, char *buffer) {
    int serialize_offset = 0;

    // Copy proc size into buffer
    int size_proc = sizeof(char) * strlen(procedure_name);
    memcpy(buffer, &size_proc, sizeof(size_proc));
    serialize_offset += sizeof(size_proc);

    // printf("The size of the second parameter is: %i\n", size_proc);

    // Copy proc name into buffer
    memcpy(buffer + serialize_offset, procedure_name, size_proc);
    serialize_offset += size_proc;

    // Copy nparams into buffer
    memcpy(buffer + serialize_offset, &nparams, sizeof(nparams));
    serialize_offset += sizeof(nparams);

    //copy args to buffer
    int i = 0;
    for(; i < nparams; i++){
        int param_size = va_arg(valist, int);
	      printf("index %i, param size %i \n", i, param_size);
        memcpy(buffer + serialize_offset, &param_size, sizeof(int));
        serialize_offset += sizeof(int);

        void *param_val = va_arg(valist, void*);
	      printf("index %i, param val %i \n", i, *(int *)param_val);
        memcpy(buffer + serialize_offset, param_val, param_size);
        serialize_offset += param_size;
    }

    return serialize_offset;
}
/*
return_type deserializeRcvBuffer(unsigned char* buffer)
{
    printf("Deserializing rcv buffer from server\n");

    int ret_size;
    void *ret_val;
    int idx = 0;

    memcpy(&ret_size, buffer, sizeof(int));
    idx += sizeof(int);

    memcpy(ret_val, buffer, ret_size);
    idx += ret_size;

    return_type ret;
    ret.return_size = ret_size;
    ret.return_val = ret_val;

    return ret;
}
*/

extern return_type make_remote_call(const char *servernameorip,
  const int serverportnumber,
  const char *procedure_name,
  const int nparams,
  ...) {

    // Create client socket
    int socket = createSocket(AF_INET, SOCK_DGRAM, 0);
    bindSocket(&socket);

    char *buffer[BUFSIZE];

    va_list valist;
    va_start(valist, nparams*2);

    serializeData(procedure_name, nparams, valist, buffer);

    // va_end(valist);

    // Create message destination address
    struct sockaddr_in serverAddress;
    memset((char*)&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverportnumber);
    struct hostent * serverLookup = getHostDetails(servernameorip); // TODO: Check if IP address or server name
    memcpy((void *)&serverAddress.sin_addr, serverLookup->h_addr_list[0], serverLookup->h_length);

    // Send message to server
    printf("Sending to server...\n");
    if (sendto(socket, buffer, sizeof(buffer),
      0, (struct sockaddr *)&serverAddress,
      sizeof(serverAddress)) < 0) {
        perror("Failed to send message.");
    }
    
    return_type ret;

    socklen_t lengthOfServerAddress = sizeof(serverAddress);
    int receivedMessage = recvfrom(socket, buffer, sizeof(buffer),
      0, (struct sockaddr *)&serverAddress,
        &lengthOfServerAddress);

    // Print message from server
    if(receivedMessage > 0) {
      printf("Received %d bytes\n", receivedMessage);
      buffer[receivedMessage] = 0;
      printf("Client received message: \"%s\"\n", buffer);
      //ret = deserializeRcvBuffer(rcvbuffer);
    }

    close(socket);

    printf("Program execution complete. \n");
    return ret;
}

int main() {
    int a = -10, b = 20;
    return_type ans = make_remote_call("ecelinux3.uwaterloo.ca",
      10000,
      "addtwo", 2,
      sizeof(int), (void *)(&a),
      sizeof(int), (void *)(&b));

//    int i = *(int *)(ans.return_val);
//    printf("client, got result: %d\n", i);

    return 0;
}
