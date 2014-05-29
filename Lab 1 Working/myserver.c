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

struct functiondatabase {
    const char *procedurename;
	int numofparams;
    fp_type fp;
} ;

struct functiondatabase fdb[100];
int fpnextavailspace = 0;

/*
//database of function pointers
struct proc_map {
    char *proc_name;
    int n_params;
    fp_type fp;
};

#define TABLE_SIZE 10
struct proc_map proc_table[TABLE_SIZE];
int index_to_insert = 0;
*/
const int clientport = 10069;

bool register_procedure(const char *procedure_name, const int nparams, fp_type fnpointer) {
	int i;
	for (i = 0; i < fpnextavailspace; i++)
	{
		int a = strcmp(fdb[i].procedurename, procedure_name);
		
		if(a == 0 && nparams == fdb[i].numofparams) {
			return false;
		}
	}
	
	fdb[fpnextavailspace].procedurename = procedure_name;
	fdb[fpnextavailspace].numofparams = nparams;
	fdb[fpnextavailspace].fp = fnpointer;
	fpnextavailspace++;
	return true;
}

/*
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
    int i = 0;
    for(; i < TABLE_SIZE; i++) {
        if((strcmp(proc_table[i].proc_name, procedure_name) == 0) 
                && (proc_table[i].n_params == nparams) )
            return false;
    }

    proc_table[index_to_insert].proc_name = procedure_name;
    proc_table[index_to_insert].n_params = nparams;
    proc_table[index_to_insert].fp = fnpointer;

    index_to_insert += 1;

    return true;
}
*/
void launch_server() {
	
	struct sockaddr_in serveraddress;      
    struct sockaddr_in remoteaddress;     
    socklen_t addrlen = sizeof(remoteaddress);         
    int recvlen;                   
    int sock;                        
    unsigned char buffer[512];     /* receive buffer */
	
    /* create a UDP socket */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    memset((char *)&serveraddress, 0, sizeof(serveraddress));
    serveraddress.sin_family = AF_INET;
    serveraddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddress.sin_port = htons(0);
    mybind(sock, ((struct sockaddr_in *)&serveraddress));
	
	int currentport = ntohs(serveraddress.sin_port);
	
	/* let client know which port to send message to */
	printf("this application is using port: %d \n", currentport);
	
    /* listening on port for client message */
    for (;;) {
		memset(buffer, 0, sizeof(buffer));
        recvlen = recvfrom(sock, (void *)buffer, sizeof(buffer), 
			0, (struct sockaddr *)&remoteaddress, &addrlen);
        if (recvlen > 0) {
			int sizeoffuncname = *(int*)buffer;
			
			unsigned char *advancedbuffer = buffer + 4;
			char receivedfuncname[sizeoffuncname];
			memset(receivedfuncname, 0, sizeof(receivedfuncname));
			char *castedadvancedbuffer = (char*)advancedbuffer;
			int i;
			for (i = 0; i < sizeoffuncname; i++) {
				receivedfuncname[i] = castedadvancedbuffer[i];
			}
			advancedbuffer = advancedbuffer + sizeoffuncname;
			
			int numofargs = *(int *)advancedbuffer; 			
			advancedbuffer = advancedbuffer + 4;
			
			/* find the function with linear search */
			fp_type myfp;
			int foundfunc = 0;
			/*
			for (i = 0; i < TABLE_SIZE; i++) {
				int a = strcmp(proc_table[i].proc_name, receivedfuncname);
				if (a == 0 && proc_table[i].n_params == numofargs) {
					myfp = proc_table[i].fp;
					foundfunc = 1;
				}
			}
			*/

			for(i = 0; i < fpnextavailspace; i++){
				if(strcmp(fdb[i].procedurename, receivedfuncname) == 0 &&
						fdb[i].numofparams == numofargs){
					myfp = fdb[i].fp;
					foundfunc = 1;
				}
			}

			/* if a remote function call is identified and the corresponding
			 * function is found in the database then
			 * make the call then return the answer */ 
			if (foundfunc == 1) {
				arg_type *athead = (arg_type*)malloc(sizeof(arg_type));
				arg_type *atcurrent = athead;
				for (i = 0; i < numofargs; i++) {
					int sizeofarg = *(int*)advancedbuffer;
					advancedbuffer = advancedbuffer + 4;
					int j;
					unsigned char *receivedargbuffer = (unsigned char*)malloc(sizeof(sizeofarg));
					for (j = 0; j < sizeofarg; j++) {
						receivedargbuffer[j] = advancedbuffer[j];
					}
					atcurrent->arg_size = sizeofarg;
					atcurrent->arg_val = (void*)receivedargbuffer;
					
					arg_type *ptr = (arg_type*)malloc(sizeof(arg_type));
					ptr->next = NULL;
					atcurrent->next = ptr;
					atcurrent = ptr;
					advancedbuffer = advancedbuffer + sizeofarg;
				}
				/* link list has been constructed 
				 * now call the function and get the returned value*/
				return_type rt;
				int m = *(int *)(athead->arg_val);
				int n = *(int *)(athead->next->arg_val);
				rt = myfp(numofargs, athead);
				
				/* send the answer back to client via udp */
				unsigned char buf[512];
				unsigned char *tmp = serialize_int(buf, rt.return_size);
				int k;
				unsigned char *castedarg = (unsigned char*)rt.return_val;
				for (k = 0; k < rt.return_size; k++) {
					tmp[k] = castedarg[k];
				}
				
				sendto(sock, buf, sizeof(buf), 0, (struct sockaddr *)&remoteaddress, addrlen);
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

unsigned char *serialize_int(unsigned char *buffer, int value) {
	/* Write little-endian int value into buffer */
	buffer[0] = value >> 0;
	buffer[1] = value >> 8;
	buffer[2] = value >> 16;
	buffer[3] = value >> 24;
	return buffer + 4;
}
