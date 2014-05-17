#include <stdio.h>
#include "ece454rpc_types.h"
#include "mybind.c"
#include <assert.h>

// Socket API
#include <sys/types.h>
#include <sys/socket.h>

// Struct sockaddr_in 
#include <netinet/in.h>
#include <netdb.h>
#include <inttypes.h>

// memset
#include <string.h>
#include <stdarg.h>

struct {
	char *proc_name;
	int num_params;
  arg_type arguments;
} proc_dec_type;

void paddr(unsigned char *a) {
    printf("%d.%d.%d.%d\n", a[0], a[1], a[2], a[3]);
}

/**
 * Sources:
 * http://www.cs.rutgers.edu/~pxk/417/notes/sockets/udp.html
 */
extern return_type make_remote_call(const char *servernameorip,
	                            	const int serverportnumber,
	                            	const char *procedure_name,
	                            	const int nparams,
				    				            ...) {

	// Result of making remote procedure call
	void *val;
	int size;
	return_type ret = {val,size};

	/**
	 * Setup a UDP datagram socket.
	 * man 2 socket
	 * man 7 ip
	 */
	int socketfd = socket(AF_INET, SOCK_DGRAM, 0); 
	if(socketfd == -1) {
		perror("Can't create socket.");
		return ret;
	}
	
	struct sockaddr_in my_addr;
	memset((char *)&my_addr, 0, sizeof(my_addr));

  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(mybind(socketfd, (struct sockaddr_in*)&my_addr) < 0 ) {
    	perror("Could't bind port to socket.");
		  return ret;	
  }

    printf("Client binded socket to port: %i \n", ntohs(my_addr.sin_port));


    // Used to retrieve IP address of the server given its name
    struct hostent *hp;     		
    struct sockaddr_in servaddr;
    char *my_message = "Test message.";
    
    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(serverportnumber);

    // Look up address of server given its name
    hp = gethostbyname(servernameorip);

    if(!hp){
    	perror("could not obtain server address");
		  return ret;	
    }

    proc_dec_type proc;
    proc.proc_name = procedure_name;
    proc.num_params = nparams;
    proc.arguments = NULL;

    arg_type list;

    va_list valist;
    va_start(valist, nparams*2);

    for(int i = 0; i < nparams*2; i++){
      if(i%2 != 0){
        proc.arguments.arg_size = va_arg(valist, int);
      } else {
        proc.arguments.arg_val = va_arg(valist, void*);

      }
    }

    char sendbuffer[1025];

    assert(sizeof proc <= sizeof sendbuffer);
   	memcpy(&proc, sendbuffer, sizeof proc);

    //put host's address into serv address structure
    memcpy((void *)&servaddr.sin_addr, hp->h_addr_list[0], hp->h_length);

  	printf("Server destination port: %i \n", ntohs(servaddr.sin_port));
  	printf("Server destination ip: ");
  	paddr((unsigned char*) hp->h_addr_list[0]);

    if(sendto(socketfd, my_message, strlen(my_message), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
    	perror("Send to server failed.");
    	return ret;
    }

    printf("Program execution complete. \n");
	  return ret;
}

int main()
{
    int a = -10, b = 20;
    return_type ans = make_remote_call("ecelinux3.uwaterloo.ca",
	                               10000,
				       				"addtwo", 2,
	                               sizeof(int), (void *)(&a),
	                               sizeof(int), (void *)(&b));
    
    int i = *(int *)(ans.return_val);
    printf("client, got result: %d\n", i);

    return 0;
}
