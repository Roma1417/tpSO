/*
 * conexiones.h
 *
 *  Created on: 3 mar. 2019
 *      Author: utnso
 */

#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include<string.h>
#include<pthread.h>

#define IP "127.0.0.1"
#define PUERTO "37227"


typedef enum{
	NEW_POKEMON = 1,
	APPEARED_POKEMON = 2,
	CATCH_POKEMON = 3,
	CAUGHT_POKEMON = 4,
	GET_POKEMON = 5,
	LOCALIZED_POKEMON = 6,
	SUSCRIPTOR = 7,
}tipo_mensaje;

typedef struct{
	tipo_mensaje id;
	t_queue* mensajes;
	t_list* suscriptores;
}cola_mensajes;

typedef struct{
	char* ip;
	char* puerto;
}suscriptor;


typedef struct{
	u_int32_t nombre_size;
	char* nombre_pokemon;
	u_int32_t pos_x;
	u_int32_t pos_y;
	u_int32_t cantidad;
}t_new_pokemon;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

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
int recibir_operacion(int);
void process_request(int cod_op, int cliente_fd);
void serve_client(int *socket);
void* serializar_paquete(t_paquete* paquete, int bytes);
void devolver_mensaje(void* payload, int size, int socket_cliente);

cola_mensajes* cola_mensajes_create(tipo_mensaje id);
void cola_mensajes_destroy(cola_mensajes* self);
suscriptor* crear_suscriptor(char* ip, char* puerto);

#endif /* CONEXIONES_H_ */
