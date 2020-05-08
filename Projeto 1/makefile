# Make file projeto 1 de redes
all: server.o cliente.o

server.o: servidor_psta.c
	gcc -pthread servidor_psta.c -o servidor
	#./server 5000

cliente.o: cliente_psta.c
	gcc -pthread cliente_psta.c -o cliente
	#./cliente
clean:
	rm -rf *.o