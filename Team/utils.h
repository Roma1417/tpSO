/*
 * utils.h
 *
 *  Created on: 3 may. 2020
 *      Author: utnso
 */

#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<pthread.h>

#define IP "127.0.0.3"
#define PUERTO "37229"

typedef enum{
	APPEARED_POKEMON = 2,
}tipo_mensaje;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct{

	u_int32_t size_pokemon;
	char* pokemon;
	u_int32_t posicion_x;
	u_int32_t posicion_y;

} t_appeared_pokemon;

typedef struct
{
	tipo_mensaje codigo_operacion;
	t_buffer* buffer;
} t_paquete;

pthread_t thread;

void* recibir_buffer(int*, int);

void iniciar_servidor(void);
void esperar_cliente(int);
void* recibir_mensaje(int socket_cliente, int* size);
void process_request(int cod_op, int cliente_fd);
void serve_client(int *socket);

#endif /* UTILS_H_ */
