all: Client Server

Client: client.c	SockLib.c
	gcc -o Client client.c	SockLib.c


Server: server.c SockLib.c
	gcc -o Server server.c SockLib.c -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -ldl