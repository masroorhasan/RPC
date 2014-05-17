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

// memset
#include <string.h>

struct proc_def {
	char *proc_name;
	int params;
};

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
	int socketfd = socket(PF_INET, SOCK_DGRAM, 0); 
	if(socketfd == -1) {
		perror("Can't create socket.");
		return ret;
	}


	struct sockaddr_in my_addr;

	// Fill a byte string with a byte value
	memset((char *)&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = PF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(mybind(socketfd, (struct sockaddr_in*)&my_addr) < 0 ) {
    	perror("Could't bind.");
		return ret;	
    }


   	//checkout: http://www.cs.rutgers.edu/~pxk/417/notes/sockets/udp.html

   	//CLIENT:
	   	//socket()
	   	//bind()
	    //sendto()		/* to server */

   	//SERVER:
   		//socket()
   		//bind() 		/* mybind() */
   		//recvfrom() 	/* blocking */
   		//(*fp)()
   		//sendto() 		/* to client */

    //might have to use struct hostent for using gethostbyname procedure
    struct hostent *hp;     		//host information
    struct sockaddr_in servaddr;	//server address

    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(serverportnumber);

    //look up address of server given its name
    hp = gethostbyname(servernameorip);
    if(!hp){
    	perror("could not obtain server address");
		return ret;	
    }

    struct proc_def proc;
    proc.proc_name = procedure_name;
    proc.params = nparams;

    char sendbuffer[1025];

    assert(sizeof proc <= sizeof sendbuffer);
   	memcpy(&proc, sendbuffer, sizeof proc);

    //put host's address into serv address structure
    memcpy((void *)&servaddr.sin_addr, hp->h_addr_list[0], hp->h_length);

    //send msg to server
    if(sendto(socketfd, (char *)&sendbuffer, strlen(sendbuffer), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
    	perror("sendto failed");
    	return ret;
    }


	//extract from recvbuffer and format into the type 'return_type' -> can be done after implementing transport on server

	return ret;
}


int main()
{
    int a = -10, b = 20;
    return_type ans = make_remote_call("exelinux3.uwaterloo.ca",
	                               5673,
				       				"addtwo", 2,
	                               sizeof(int), (void *)(&a),
	                               sizeof(int), (void *)(&b));
    int i = *(int *)(ans.return_val);
    printf("client, got result: %d\n", i);

    return 0;
}
