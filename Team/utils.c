/*
 * utils.c
 *
 *  Created on: 6 may. 2020
 *      Author: utnso
 */

#include "utils.h"

void iniciar_servidor(void)
{
	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(IP, PUERTO, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    while(1)
    	esperar_cliente(socket_servidor);
}

void esperar_cliente(int socket_servidor)
{
	struct sockaddr_in dir_cliente;

	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	pthread_create(&thread,NULL,(void*)serve_client,&socket_cliente);
	pthread_detach(thread);
}

void serve_client(int* socket)
{
	int cod_op;
	if(recv(*socket, &cod_op, sizeof(int), MSG_WAITALL) == -1)
		cod_op = -1;
	process_request(cod_op, *socket);
}

void process_request(int cod_op, int cliente_fd) {
	int size;
	void* msg;
		switch (cod_op) {
		case APPEARED_POKEMON:{
			//msg = recibir_mensaje(cliente_fd, &size);

			int size_buffer = 0;

			recv(cliente_fd, &size_buffer, sizeof(int), MSG_WAITALL);

			t_appeared_pokemon* appeared_pokemon = malloc(sizeof(t_appeared_pokemon));

			recv(cliente_fd, &(appeared_pokemon->size_pokemon), sizeof(u_int32_t), MSG_WAITALL);
			recv(cliente_fd, appeared_pokemon->pokemon, appeared_pokemon->size_pokemon, MSG_WAITALL);
			recv(cliente_fd, &(appeared_pokemon->posicion_x), sizeof(u_int32_t), MSG_WAITALL);
			recv(cliente_fd, &(appeared_pokemon->posicion_y), sizeof(u_int32_t), MSG_WAITALL);

			printf("Llego un mensaje: %s\n", appeared_pokemon->pokemon);
			printf("size_pokemon: %d\n", appeared_pokemon->size_pokemon);
			printf("posicion_x: %d\n", appeared_pokemon->posicion_x);
			printf("posicion_y: %d\n", appeared_pokemon->posicion_y);

			//free(msg);
			break;
		}
		case 0:
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
		}
}

void* recibir_mensaje(int socket_cliente, int* size)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

