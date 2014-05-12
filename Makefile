objects := $(patsubst %.c,%.o,$(wildcard *.c))

a.out: libstubs.a myclient.o myserver.o
	gcc myclient.o myserver.o -L. -lstubs -o a.out

libstubs.a: server_stub.o client_stub.o
	ar r libstubs.a server_stub.o client_stub.o

$(objects): %.o: %.c ece454rpc_types.h
	gcc -c $< -o $@

clean:
	rm -rf a.out *.o core *.a
