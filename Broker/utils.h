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


void iniciar_servidor(void);
void esperar_cliente(int);
void process_request(int, int);
void serve_client(int*);
void* serializar_paquete(t_paquete*, size_t);;
char* consultar_config_por_string(char*, char*);
int recibir_entero(int);
void* recibir_cadena(int, int*);
tipo_mensaje obtener_tipo_mensaje(char* tipo);
char* obtener_tipo_mensaje_string(tipo_mensaje);
char* consultar_config_por_string(char*, char*);
void finalizar_servidor();
void notificar_id_suscriptor(t_suscriptor*, tipo_mensaje);
u_int32_t enviar_mensaje(t_paquete*, u_int32_t);

void enviar_a_suscriptor(t_mensaje*, t_suscriptor*);
void esperar_confirmacion(t_suscriptor*);
void enviar_a_suscriptores(t_mensaje*, t_list*);
void agregar_suscriptor(u_int32_t, t_cola_mensajes*);
t_suscriptor* actualizar_suscriptor(u_int32_t, t_cola_mensajes*, u_int32_t);
bool es_cola_correlativa(tipo_mensaje);

#endif /* CONEXIONES_H_ */
