# Make file projeto 1 de redes
all: server.o cliente.o

server.o: TCP-Servidor-Thread.c
	gcc -pthread TCP-Servidor-Thread.c -o server
	#./server 5000

cliente.o: TCP-Cliente-Thread.c
	gcc -pthread TCP-Cliente-Thread.c -o cliente
	#./cliente
clean:
	rm -rf *.o