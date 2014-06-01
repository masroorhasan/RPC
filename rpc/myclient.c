/**
 * References:
 *
 * http://www.cs.rutgers.edu/~pxk/417/notes/sockets/udp.html
 * http://stackoverflow.com/questions/9778806/serializing-a-class-with-a-pointer-in-c
 * http://stackoverflow.com/questions/504810/how-do-i-find-the-current-machines-full-hostname-in-c-hostname-and-domain-info
 *
 * Coding Style:
 *
 * http://www.cs.swarthmore.edu/~newhall/unixhelp/c_codestyle.html
 */

#include <stdio.h>
#include "rpc_types.h"

int main() {
    int a = -10, b = 20;
    return_type ans = make_remote_call("ecelinux3.uwaterloo.ca",
                                   10001, "addtwo", 2,
                                   sizeof(int), (void *)(&a),
                                   sizeof(int), (void *)(&b));
    int result = *(int *)(ans.return_val);

    printf("Client, got result: %d\n", result);

    ans = make_remote_call("ecelinux3.uwaterloo.ca",
                                   10001, "addtwo1", 2,
                                   sizeof(int), (void *)(&a),
                                   sizeof(int), (void *)(&b));
		printf("Client finished making unregistered function call.\n");


    ans = make_remote_call("ecelinux3.uwaterloo.ca",
                                   10001, "", 2,
                                   sizeof(int), (void *)(&a),
                                   sizeof(int), (void *)(&b));


		printf("Client finished making no procedure name function call.\n");


    ans = make_remote_call("ecelinux3.uwaterloo.ca",
                                   10001, "warrenthefuckingmansmith", 2,
                                   sizeof(int), (void *)(&a),
                                   sizeof(int), (void *)(&b));

    result = *(int *)(ans.return_val);

    printf("Client, got result: %d\n", result);

    return 0;
}
