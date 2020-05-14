all: server
server: broker.c utils.c
	gcc broker.c utils.c cola_mensajes.c -o broker -lcommons -lpthread 
