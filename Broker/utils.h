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
#include<commons/string.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include<time.h>
#include<string.h>
#include<pthread.h>

#include "collections.h"
#include "memory.h"




pthread_t thread;
t_memoria* memoria;

char* ip ;
char* puerto;


void iniciar_servidor(void);
void esperar_cliente(int);
void process_request(int, int);
void serve_client(int*);
void* serializar_paquete(t_paquete*, size_t);
int recibir_entero(int);
void* recibir_cadena(int, int*);
char* recibir_string(int socket_cliente, u_int32_t* size);
tipo_mensaje obtener_tipo_mensaje(char* tipo);
char* obtener_tipo_mensaje_string(tipo_mensaje);
void finalizar_servidor();
void notificar_id_suscriptor(t_suscriptor*, tipo_mensaje);
u_int32_t enviar_mensaje(t_paquete*, u_int32_t);

void enviar_a_suscriptor(t_particion*, t_suscriptor*);
void esperar_confirmacion(t_suscriptor*);
void enviar_a_suscriptores(t_particion*, t_list*);
void agregar_suscriptor(u_int32_t, uint32_t);
t_suscriptor* actualizar_suscriptor(u_int32_t, uint32_t, u_int32_t);
bool confirmo_mensaje(t_suscriptor*, t_particion*);

bool es_cola_correlativa(tipo_mensaje);

#endif /* CONEXIONES_H_ */
