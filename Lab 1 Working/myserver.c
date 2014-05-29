/**
 * References:
 *
 * http://www.cs.rutgers.edu/~pxk/417/notes/sockets/udp.html
 * http://stackoverflow.com/questions/9778806/serializing-a-class-with-a-pointer-in-c
 *
 * Coding Style:
 *
 * http://www.cs.swarthmore.edu/~newhall/unixhelp/c_codestyle.html
 */

#include "ece454rpc_types.h"
#include "mybind.c"

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>

// Database structure to store procedure properties
struct proc_map_db {
    const char *proc_name;
      int n_params;
    fp_type fp;
};

int ret_int;
return_type r;
const int clientport = 10069;

// Database declaration
struct proc_map_db proc_db[100];
int proc_db_index = 0;

/**
 * Creates and a new socket instance and returns socket identifier.
 */
int create_socket(const int domain, const int type, const int protocol) {

    int socketDescriptor = socket(domain, type, protocol);

    if (socketDescriptor == -1) {
        perror("Failed to create socket.");
        exit(0);
    }

    printf("Created socket.\n");
    return socketDescriptor;
}

/**
 * Binds a socket identifier to host address.
 */
void bind_socket(int socket) {

    struct sockaddr_in myAddress;

    memset((char *)&myAddress, 0, sizeof(myAddress));
    myAddress.sin_family = AF_INET;
    myAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    myAddress.sin_port = htons(0);
    // Assign port
    if(mybind(socket, (struct sockaddr_in*)&myAddress) < 0 ) {
        perror("Could't bind port to socket.");
        exit(0);
    }

    printf("Binded socket to port: %i \n", ntohs(myAddress.sin_port));
}

/**
 * Registers a new procedure to the procedure database.
 */
bool register_procedure(const char *procedure_name, const int nparams, fp_type fnpointer) {

    int i = 0;
    while (i < proc_db_index){
        if( (strcmp( proc_db[i].proc_name, procedure_name) == 0)
            && (nparams == proc_db[i].n_params) ) {
            return false;
        }
        i++;
    }

    proc_db[proc_db_index].proc_name = procedure_name;
    proc_db[proc_db_index].n_params = nparams;
    proc_db[proc_db_index].fp = fnpointer;
    proc_db_index++;

    return true;
}

/**
 * Serializes int values into a character buffer.
 */
unsigned char *int_serialize(unsigned char *buffer, int value) {

    int shift = 0;
    const int shift_eight = 8;
    const int shift_sixteen = 16;
    const int shift_twentyfour = 24;

    buffer[shift] = value >> shift;
    buffer[++shift] = value >> shift_eight;
    buffer[++shift] = value >> shift_sixteen;
    buffer[++shift] = value >> shift_twentyfour;

    return buffer + shift + 1;
}

/**
 * Deserializes buffer received from client.
 */
return_type deserialize(unsigned char * buffer){

    return_type ret;

    // Gets size of procedure name
    int proc_size = *(int*)buffer;
    unsigned char *aliased_buff = buffer + sizeof(int);

    // Gets actual procedure name
    char recv_proc_name[proc_size];
    memset(recv_proc_name, 0, sizeof(recv_proc_name));
    char *dummyaliased_buff = (char*)aliased_buff;

    int i = 0;
    while (i < proc_size) {
        recv_proc_name[i] = dummyaliased_buff[i];
        i++;
    }

    aliased_buff += proc_size;

    // Gets number of parameters for procedure
    int recv_num_args = *(int *)aliased_buff;
    aliased_buff = aliased_buff + 4;

    // Find the procedure from the database
    fp_type fp;
    arg_type *arg_list;
    int proc_found = 0;
    
    i = 0;
    while (i < proc_db_index){
        if(strcmp(proc_db[i].proc_name, recv_proc_name) == 0 &&
                proc_db[i].n_params == recv_num_args){
            fp = proc_db[i].fp;
            proc_found = 1;
        }

        i++;
    }

    // Proceeds to get arguments if procedure was found in database
    if (proc_found == 1) {

        arg_type *head_node = (arg_type*)malloc(sizeof(arg_type));
        arg_type *curr_node = head_node;

        // Build linked list of arguments as we unpack from buffer
        i = 0;
        while (i < recv_num_args) {
            int recv_arg_size = *(int *)aliased_buff;
            aliased_buff += sizeof(int);
            
            unsigned char *recv_arg_val = (unsigned char*)malloc(sizeof(recv_arg_size));

            int j = 0;
            while (j < recv_arg_size) {
                recv_arg_val[j] = aliased_buff[j];
                j++;
            }

            arg_type *ptr = (arg_type*)malloc(sizeof(arg_type));
            ptr->next = NULL;

            curr_node->arg_size = recv_arg_size;
            curr_node->arg_val = (void*)recv_arg_val;

            curr_node->next = ptr;
            curr_node = ptr;

            aliased_buff += recv_arg_size;
            i++;
        }

        arg_list = head_node;
    }

    // Call function ptr to compute and return result
    ret = fp(recv_num_args, arg_list);
    return ret;
}

/**
 * Main loop that keeps server running and processing incoming procedure calls.
 */
void launch_server() {

    struct sockaddr_in serv_addr;
    struct sockaddr_in remote_addr;
    socklen_t addr_len = sizeof(remote_addr);
    int socket;

    // Creates a UDP socket
    socket = create_socket(AF_INET, SOCK_DGRAM, 0);
    bind_socket(socket);

    int received_size;
      unsigned char buffer[512];

    for (;;) {
        memset(buffer, 0, sizeof(buffer));

        // Populate buffer with data from client
        received_size = recvfrom(socket, (void *)buffer, sizeof(buffer),
            0, (struct sockaddr *)&remote_addr, &addr_len);

        // If we recieved data from client, move onto deserializing it
        if (received_size > 0) {

            return_type ret;
            ret = deserialize(buffer);

            // Process and send result back to client
            unsigned char send_buf[512];
            unsigned char *tmp = int_serialize(send_buf, ret.return_size);
            unsigned char *castedarg = (unsigned char*)ret.return_val;

            int k = 0;
            while (k < ret.return_size) {
                tmp[k] = castedarg[k];
                k++;
            }

            sendto(socket, send_buf, sizeof(send_buf), 0, (struct sockaddr *)&remote_addr, addr_len);
        }
    }
}

return_type add(const int nparams, arg_type* a) {
    
    if(nparams != 2) {
        /* Error! */
        r.return_val = NULL;
        r.return_size = 0;
        return r;
    }

    if(a->arg_size != sizeof(int) ||
       a->next->arg_size != sizeof(int)) {
       printf("arg_size is %d, next_arg_size is %d", a->arg_size, a->next->arg_size);
        /* Error! */
        r.return_val = NULL;
        r.return_size = 0;
        return r;
    }

    int i = *(int *)(a->arg_val);
    int j = *(int *)(a->next->arg_val);

    printf("i is %d , j is %d \n", i, j);

    ret_int = i+j;
    r.return_val = (void *)(&ret_int);
    r.return_size = sizeof(int);

    return r;
}

int main() {
    register_procedure("addtwo", 2, add);

    launch_server();

    /* should never get here, because
       launch_server(); runs forever. */

    return 0;
}
