/**
 * Sources:
 * http://www.cs.rutgers.edu/~pxk/417/notes/sockets/udp.html
 */

#include <stdio.h>
#include <stdlib.h>
#include "ece454rpc_types.h"
#include "mybind.c"

// Socket
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// memset
#include <string.h>

#define BUFSIZE 2048

int ret_int;
return_type r;

//database of function pointers
struct proc_map {
    char *proc_name;
    fp_type fp;
};

#define TABLE_SIZE 10
struct proc_map proc_table[TABLE_SIZE];
int index_to_insert = 0;

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
    myAddress.sin_family = AF_INET;
	myAddress.sin_port = 0;
    // Assign port
    if(mybind(*socket, (struct sockaddr_in*)&myAddress) < 0 ) {
        perror("Could't bind port to socket.");
        exit(0);
    }

    printf("Binded socket to port: %i \n", ntohs(myAddress.sin_port));
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
    if(procedure_name == NULL) return false;
    if(nparams < 2) return false;
    if(index_to_insert > TABLE_SIZE) return false;

    //register function in proc_table
    //db looked up by name, i.e. procedure_name

    proc_table[index_to_insert].proc_name = procedure_name;
    proc_table[index_to_insert].fp = fnpointer;

    index_to_insert += 1;

    return true;
}

return_type deserializeBuffer(unsigned char *rcvbuffer) {
    size_t idx = 0;
    //read sizeof(int) and get proc_size_value
    int proc_size;
    memcpy(&proc_size, &rcvbuffer[idx], sizeof(int));
    idx += sizeof(proc_size);
    	
    printf("deserialized proc size: %i\n", proc_size);
    
    //printf("reading off buffer: %s\n", rcvbuffer[idx+1]);

    //read sizeof(proc_size_value) to get procname
    char *proc_name[proc_size];
    memcpy(&proc_name, &rcvbuffer[idx], sizeof(proc_name));
    idx += sizeof(proc_name);

    printf("deserialized proc name: %s\n", proc_name);

    //read sizeof(int) and get nparams
    int num_params;
    memcpy(&num_params, &rcvbuffer[idx], sizeof(num_params));
    idx += sizeof(num_params);
    //read sizeof(int) * 2 to get size and param value from 
    arg_type list;
    memcpy(&list, &rcvbuffer[idx], sizeof(list));
    idx += sizeof(list);

    int i = 0;
    return_type ret;
    for(; i < TABLE_SIZE; i++) {
        if(proc_table[i].proc_name == proc_name) {
            //can grab func ptr to call function with known signature
            // proc_table[i].fp 
            ret = (* proc_table[i].fp)(num_params, &list);
            break;
        }
    }

    printf("return value: %i \n", *(int *)ret.return_val);

    return ret;
}

/* launch_server() -- used by the app programmer's server code to indicate that
 * it wants start receiving rpc invocations for functions that it registered
 * with the server stub. */
void launch_server() {

    int socket = createSocket(AF_INET, SOCK_DGRAM, 0);
    bindSocket(&socket);

    struct sockaddr_in remoteAddress;
    socklen_t remoteAddressLength = sizeof(remoteAddress);

    unsigned char receiveBuffer[BUFSIZE];
    int receivedSize;

    for (;;) {
        receivedSize = recvfrom(socket, receiveBuffer, BUFSIZE,
            0, (struct sockaddr *)&remoteAddress, &remoteAddressLength);

        printf("Received %d bytes\n", receivedSize);

        if (receivedSize > 0) {
            receiveBuffer[receivedSize] = 0;
            //deserialize rcvbuffer, return proc_name and args
            deserializeBuffer(receiveBuffer);
            //compute result
            //take result and sendto() client            

            printf("Received Message: \"%s\"\n", receiveBuffer);
        }
    }

}

int main() {
    register_procedure("addtwo", 2, add);
    //register other procedures here

    launch_server();

    return 0;
}
