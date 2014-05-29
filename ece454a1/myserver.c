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
#include "ece454rpc_types.h"

int ret_int;
return_type r;

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

    return 0;
}
