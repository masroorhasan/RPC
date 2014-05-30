objects := $(patsubst %.c,%.o,$(wildcard *.c))

libstubs.a: server_stub.o client_stub.o mybind.o
	ar r libstubs.a server_stub.o client_stub.o mybind.o

$(objects): %.o: %.c ece454rpc_types.h
	gcc -c $< -o $@

clean:
	rm -rf a.out *.o core *.a
