#include <stdio.h>
#include "ece454rpc_types.h"

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

return_type add(const int nparams, arg_type* a)
{
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

/* launch_server() -- used by the app programmer's server code to indicate that
 * it wants start receiving rpc invocations for functions that it registered
 * with the server stub. */
void launch_server() 
{
    //look up 'database' of all functions that are registered in it and,
    //invoke the appropriate application procedure

    // while(1){
    //     //service client requests
    // }
}

int main() {
    register_procedure("addtwo", 2, add);

    launch_server();

    /* should never get here, because
       launch_server(); runs forever. */

    return 0;
}
