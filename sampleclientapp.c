#include <stdio.h>
#include "ece454rpc_types.h"


//using socket API
#include <sys/socket.h>

//to use sockaddr_in struct
#include <netinet/in.h>

#include <assert.h>

struct proc_def {
	char *proc_name;
	int params;
};

extern return_type make_remote_call(const char *servernameorip,
	                            	const int serverportnumber,
	                            	const char *procedure_name,
	                            	const int nparams,
				    				...)
{

	void *val = NULL;
	int size = NULL; 
	return_type ret = {val,size};

	//build transport here
	//translate call to a network message
	//using UDP, socket() -- man 2 socket, for socket API
	int socketfd; 
	struct sockaddr_in serv_addr;

	//socket(domain, type, protocol)
	//domain: PF_INET (IP)
	//type: SOCK_DGRAM (datagram service)
	if(socketfd = socket(PF_INET, SOCK_DGRAM, 0) < 0){
		perror("cannot create socket");
		return ret;
	}

	//define domain, address and port to send rpc to
	memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = PF_INET;
    serv_addr.sin_port = serverportnumber; 
    serv_addr.sin_addr.s_addr = inet_addr(servernameorip);

    //int inet_pton(int af, const char *src, void *dst);
    if(inet_pton(PF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\n inet_pton error occured\n");
        return ret;
    } 

    //NOTE: checkout bind() 

    struct proc_def proc;
    proc.proc_name = procedure_name;
    proc.params = nparams;

    char sendbuffer[1025];

    assert(sizeof proc <= sizeof sendbuffer);
   	memcpy(&proc, sendbuffer, sizeof proc);

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

    /*
	//connect to server
	//connect(int socket, const struct sockaddr *address,socklen_t address_len);
    if(connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
       printf("\n Error : Connect Failed \n");
       return ret;
    } 

    //write() to server, send the name and params and procedure to invoke on server
    struct proc_def proc;
    proc.proc_name = procedure_name;
    proc.params = nparams;

    char sendbuffer[1025];

    assert(sizeof proc <= sizeof sendbuffer);
   	memcpy(&proc, sendbuffer, sizeof proc);
    
    
    write(socketfd, sendbuffer, strlen(sendbuffer));

	//read() from server, read what server sent back as a result of invoking the procedure 
	int n;
	char recvbuffer[1025];
	while((n = read(socketfd, recvbuffer, sizeof(recvbuffer))) > 0) {
		recvbuffer[n] = 0;
		if(fputs(recvbuffer, stdout) == EOF){
			printf("\n fputs error \n");
		}
	}

	if(n < 0) {
		printf("\n Read error \n");
	}
	*/

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
