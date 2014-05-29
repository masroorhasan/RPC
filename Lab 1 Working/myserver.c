#include <stdio.h>
#include "ece454rpc_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include "mybind.c"
int ret_int;
return_type r;

unsigned char *serialize_int(unsigned char *buffer, int value);

struct proc_map_db {
    const char *proc_name;
	int n_params;
    fp_type fp;
} ;

struct proc_map_db proc_db[100];
int proc_db_index = 0;

const int clientport = 10069;

bool register_procedure(const char *procedure_name, const int nparams, fp_type fnpointer) {
	int i = 0;
	for (; i < proc_db_index; i++)
	{	
		if( (strcmp( proc_db[i].proc_name, procedure_name) == 0)
					&& (nparams == proc_db[i].n_params) ) {
			return false;
		}
	}
	
	proc_db[proc_db_index].proc_name = procedure_name;
	proc_db[proc_db_index].n_params = nparams;
	proc_db[proc_db_index].fp = fnpointer;
	proc_db_index++;
	
	return true;
}

unsigned char *serialize_int(unsigned char *buffer, int value) {
	/* Write little-endian int value into buffer */
	buffer[0] = value >> 0;
	buffer[1] = value >> 8;
	buffer[2] = value >> 16;
	buffer[3] = value >> 24;
	return buffer + 4;
}

void launch_server() {
	
	struct sockaddr_in serv_addr;      
    struct sockaddr_in remote_addr;     
    socklen_t addr_len = sizeof(remote_addr);         
    int received_size;                   
    int sock;                        
    unsigned char buffer[512];
	
    /* create a UDP socket */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(0);
    mybind(sock, ((struct sockaddr_in *)&serv_addr));
	
	int port_curr = ntohs(serv_addr.sin_port);
	
	/* let client know which port to send message to */
	printf("this application is using port: %d \n", port_curr);
	
    /* listening on port for client message */
    for (;;) {
		memset(buffer, 0, sizeof(buffer));
        received_size = recvfrom(sock, (void *)buffer, sizeof(buffer), 
			0, (struct sockaddr *)&remote_addr, &addr_len);
        if (received_size > 0) {
			int proc_size = *(int*)buffer;
			
			unsigned char *aliased_buff = buffer + 4;
			char recv_proc_name[proc_size];
			memset(recv_proc_name, 0, sizeof(recv_proc_name));
			char *dummyaliased_buff = (char*)aliased_buff;
			int i = 0;
			for (; i < proc_size; i++) {
				recv_proc_name[i] = dummyaliased_buff[i];
			}
			aliased_buff = aliased_buff + proc_size;
			
			int recv_num_args = *(int *)aliased_buff; 			
			aliased_buff = aliased_buff + 4;
			
			/* find the function with linear search */
			fp_type fp;
			int proc_found = 0;

			for(i = 0; i < proc_db_index; i++){
				if(strcmp(proc_db[i].proc_name, recv_proc_name) == 0 &&
						proc_db[i].n_params == recv_num_args){
					fp = proc_db[i].fp;
					proc_found = 1;
				}
			}

			/* if a remote function call is identified and the corresponding
			 * function is found in the database then
			 * make the call then return the answer */ 
			if (proc_found == 1) {
				arg_type *head_node = (arg_type*)malloc(sizeof(arg_type));
				arg_type *curr_node = head_node;
				i = 0;
				for (; i < recv_num_args; i++) {
					int recv_arg_size = *(int*)aliased_buff;
					aliased_buff = aliased_buff + 4;
					int j;
					unsigned char *recv_arg_val = (unsigned char*)malloc(sizeof(recv_arg_size));
					for (j = 0; j < recv_arg_size; j++) {
						recv_arg_val[j] = aliased_buff[j];
					}
					curr_node->arg_size = recv_arg_size;
					curr_node->arg_val = (void*)recv_arg_val;
					
					arg_type *ptr = (arg_type*)malloc(sizeof(arg_type));
					ptr->next = NULL;
					curr_node->next = ptr;
					curr_node = ptr;
					aliased_buff = aliased_buff + recv_arg_size;
				}
				/* link list has been constructed 
				 * now call the function and get the returned value*/
				return_type ret;
				int m = *(int *)(head_node->arg_val);
				int n = *(int *)(head_node->next->arg_val);

				ret = fp(recv_num_args, head_node);
				
				/* send the answer back to client via udp */
				unsigned char send_buf[512];
				unsigned char *tmp = serialize_int(send_buf, ret.return_size);
				int k;
				unsigned char *castedarg = (unsigned char*)ret.return_val;
				for (k = 0; k < ret.return_size; k++) {
					tmp[k] = castedarg[k];
				}
				
				sendto(sock, send_buf, sizeof(send_buf), 0, (struct sockaddr *)&remote_addr, addr_len);
			}
        }
    }
}

return_type add(const int nparams, arg_type* a)
{
	printf("running the function add \n, nparams = %d", nparams);
    if(nparams != 2) {
	printf("nparams is not 2 \n");
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
