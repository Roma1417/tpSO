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
#include<time.h>
#include<string.h>
#include<pthread.h>
#include"cola_mensajes.h"



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
void* recibir_mensaje(int, int*);
int recibir_operacion(int);
void process_request(int, int);
void serve_client(int*);
void* serializar_paquete(t_paquete*, int);
void devolver_mensaje(void*, int, int);
char* consultar_config_por_string(char*, char*);
int recibir_entero(int);
void* recibir_cadena(int, int*);
tipo_mensaje obtener_tipo_mensaje(char* tipo);
char* obtener_tipo_mensaje_string(tipo_mensaje);
char* consultar_config_por_string(char*, char*);
void finalizar_servidor();

#endif /* CONEXIONES_H_ */
