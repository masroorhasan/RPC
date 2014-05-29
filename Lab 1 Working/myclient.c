/**
 * References:
 * 
 * http://www.cs.rutgers.edu/~pxk/417/notes/sockets/udp.html
 * http://stackoverflow.com/questions/9778806/serializing-a-class-with-a-pointer-in-c
 */

#include "ece454rpc_types.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

const int client_port = 10069;
const int buffer_size = 512;

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
 * Makes a remote procedure call on the specific server.
 */
return_type make_remote_call(const char *servernameorip,
								const int serverportnumber,
								const char *procedure_name,
								const int nparams, ...) {

    // Create server address
    struct sockaddr_in server_socket_address;
    memset((char *)&server_socket_address, 0, sizeof(server_socket_address));
    server_socket_address.sin_port = htons(serverportnumber);
    server_socket_address.sin_family = AF_INET;

    // Lookup the server's IP using the hostname provided
    struct hostent *server_ip_address;
    server_ip_address = gethostbyname(servernameorip);
    memcpy((void *)&server_socket_address.sin_addr, server_ip_address->h_addr_list[0],
        server_ip_address->h_length);

    // Create client socket and bind address to it
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in client_socket_address;
	memset((char *)&client_socket_address, 0, sizeof(client_socket_address));
	client_socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
	client_socket_address.sin_family = AF_INET;
    client_socket_address.sin_port = htons(client_port);
	bind(client_socket, (struct sockaddr *)&client_socket_address, sizeof(client_socket_address));

    /**
     * Pack data into buffer
     *
     * Buffer serialization format:
     * [functionSize | functionName]
     * [argumentSize | argument]
     * [argumentSize | argument]
     */

    unsigned char buffer[buffer_size];
    memset(buffer, 0, sizeof(buffer));

	// Pack function name and size in buffer
  	int function_name_size = sizeof(procedure_name);
    unsigned char *serial_result = int_serialize(buffer, function_name_size);
    unsigned char *function_name = (unsigned char*)procedure_name;

    int i = 0;
    while (i < function_name_size) {
        serial_result[i] = function_name[i];
        i++;
    }

  	serial_result = serial_result + function_name_size;
  	serial_result = int_serialize(serial_result, nparams);

	// Pack argument size and argument in buffer
    va_list var_args;
    va_start(var_args, nparams);
    i = 0;

    while (i < nparams) {
        // Put argument size in buffer
    	int arg_size = va_arg(var_args, int);
		serial_result = int_serialize(serial_result, arg_size);

    	// Put argument in buffer
    	void *arg = va_arg(var_args, void *);
    	unsigned char *char_arg = (unsigned char*)arg;
		printf("Serializing argument: %d \n", *(int*)char_arg);

        int j = 0;
        while (j < arg_size) {
			serial_result[j] = char_arg[j];
            j++;
		}
		printf("Sending argument: %d \n", *(int*)serial_result);

		serial_result = serial_result + arg_size;
        i++;
    }

    va_end(var_args);

    /**
     * Make remote procedure call
     */
	sendto(client_socket, buffer, sizeof(buffer), 0,
		(struct sockaddr *)&server_socket_address, sizeof(server_socket_address));

	/**
     * Listen for the result of remote procedure call
     */
	struct sockaddr_in remote_address;
    socklen_t addrlen = sizeof(remote_address);

	unsigned char receive_buffer[buffer_size];
	while (1) {
		int receive_length = recvfrom(client_socket, (void *)receive_buffer, sizeof(receive_buffer),
			0, (struct sockaddr *)&remote_address, &addrlen);

        if (receive_length > 0) {
			// Got a good message! Woot!
			unsigned char *return_value_buffer = receive_buffer + 4;
            int return_size = *(int*)receive_buffer;
            unsigned char return_value[return_size];
            
            int k = 0;
            while (k < return_size) {
				return_value[k] = return_value_buffer[k];
			    k++;
            }

			return_type rt;
			memset((unsigned char *)&rt, 0, sizeof(rt));
			rt.return_val = return_value;
            rt.return_size = return_size;
			
			return rt;
		}
	}
}

int main() {
    int a = -10, b = 20;
    return_type ans = make_remote_call("ecelinux3.uwaterloo.ca",
	                               10004, "addtwo", 2,
	                               sizeof(int), (void *)(&a),
	                               sizeof(int), (void *)(&b));
    int result = *(int *)(ans.return_val);

    printf("Client, got result: %d\n", result);

    return 0;
}
