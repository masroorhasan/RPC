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

#define BUFSIZE 512

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
    printf("here in addtwo function\n");
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

return_type deserializeBuffer(unsigned char *buffer) {
    printf("deserializeBuffer");
 
    return_type ret;
    int deserialize_proc_size = 0;
    int deserialize_nparams = 0;
    int deserialize_offset = 0;

    //extract proc name size
    memcpy(&deserialize_proc_size, buffer, sizeof(int));
    deserialize_offset += sizeof(int);
    printf("deserialize proc size: %i\n", deserialize_proc_size);

    //extract proc name
    char *deserialize_proc_name = malloc(deserialize_proc_size);
    memcpy(deserialize_proc_name, buffer + deserialize_offset, deserialize_proc_size);
    deserialize_offset += deserialize_proc_size;
    printf("deserialize: The fourth thing from the buffer is %s\n", deserialize_proc_name);

    //extract nparams
    memcpy(&deserialize_nparams, buffer + deserialize_offset, sizeof(int));
    deserialize_offset += sizeof(int);
    printf("deserialize: The fifth thing from the buffer is %i\n", deserialize_nparams);

    //linked list
    arg_type *head = (arg_type*)malloc(sizeof(arg_type));
    arg_type *current = head;
    int i = 0;
    for (; i < deserialize_nparams; i++) {
        int arg_size;
        memcpy(&arg_size, buffer + deserialize_offset, sizeof(int));
        printf("index %i, arg size: %i\n", i, arg_size);
        deserialize_offset += sizeof(int);

        void *arg_val;
        memcpy(arg_val, buffer + deserialize_offset, arg_size);
        printf("index %i, arg val: %i\n", i, *(int *)arg_val);
        deserialize_offset += arg_size;

        current->arg_size = arg_size;
        current->arg_val = arg_val;
        
        arg_type *ptr = (arg_type*)malloc(sizeof(arg_type));
        ptr->next = NULL;
        current->next = ptr;
        current = ptr;
    }

    int n = *(int *)(head->arg_val);
    int m = *(int *)(head->next->arg_val);

    printf("n %i\n", n); 
    printf("m %i\n", m); 
    i = 0;
    for(; i < TABLE_SIZE; i++){
        if(strcmp(proc_table[i].proc_name, deserialize_proc_name) == 0){
            ret = (proc_table[i].fp)(deserialize_nparams, head);
            break;
        }
    }
    free(head);
    printf("return val %i \n", *(int *)ret.return_val);
    return ret;
}

void serializeSendBuffer(unsigned char *buffer, return_type ret)
{
    int idx = 0;
    int ret_size = ret.return_size;
    void *ret_val = ret.return_val;
    printf("serializing procedure result to send back to client\n");

    printf("ret_size passed in: %i\n", ret_size);
    printf("ret_val passed in: %i\n", *(int *)ret_val);

    memcpy(buffer, &ret_size, sizeof(int) );
    printf("return size %i \n", *(int *)buffer);
    idx += sizeof(int);

    memcpy(buffer+idx, ret_val, ret_size);
    printf("return val %i \n", *(int *)(buffer+idx));
    idx += ret_size;
}


/* launch_server() -- used by the app programmer's server code to indicate that
 * it wants start receiving rpc invocations for functions that it registered
 * with the server stub. */
void launch_server() {

    // int socket = createSocket(AF_INET, SOCK_DGRAM, 0);
    // bindSocket(&socket);

    struct sockaddr_in serveraddress;
    struct sockaddr_in remoteAddress;
    socklen_t remoteAddressLength = sizeof(remoteAddress);
//    int remoteAddressLength = sizeof(remoteAddress);

    unsigned char receiveBuffer[BUFSIZE];
    int receivedSize;      

    /* create a UDP socket */
    // int socket = socket(AF_INET, SOCK_DGRAM, 0);
    int socket = createSocket(AF_INET, SOCK_DGRAM, 0);
    memset((char *)&serveraddress, 0, sizeof(serveraddress));
    serveraddress.sin_family = AF_INET;
    serveraddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddress.sin_port = htons(0);
    mybind(socket, (struct sockaddr *)&serveraddress);
    
    int currentport = ntohs(serveraddress.sin_port);
    
    /* let client know which port to send message to */
    printf("this application is using port: %d \n", currentport);

    for (;;) {
        memset(receiveBuffer, 0, sizeof(receiveBuffer));
        receivedSize = recvfrom(socket, (void *)receiveBuffer, BUFSIZE,
            0, (struct sockaddr *)&remoteAddress, &remoteAddressLength);

        printf("Received %d bytes\n", receivedSize);

        if (receivedSize > 0) {
            // receiveBuffer[receivedSize] = 0;
            //deserialize rcvbuffer, return proc_name and args
		unsigned char *buff = receiveBuffer;
	       return_type ret = deserializeBuffer(buff);
	       memset(receiveBuffer, 0, sizeof(receiveBuffer));
	       serializeSendBuffer(receiveBuffer, ret);
        }


	printf("Sending to client..\n");

	//printf("Remote address: %s", inet_ntoa(remoteAddress.sin_addr));
	//printf("%s:%d of addr length %d\n",inet_ntoa(remoteAddress.sin_addr),remoteAddress.sin_port,remoteAddressLength);
        //printf("Calling sendto()..\n");
	sendto(socket, receiveBuffer, sizeof(receiveBuffer), 0, (struct sockaddr *)&remoteAddress, remoteAddressLength);
        memset(receiveBuffer, 0, sizeof(receiveBuffer));

    }

}

int main() {
    register_procedure("addtwo", 2, add);
    //register other procedures here

    launch_server();

    return 0;
}
