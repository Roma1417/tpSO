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




pthread_t thread;


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
void notificar_suscriptor(t_suscriptor*, tipo_mensaje);
void enviar_mensaje(t_paquete*, int);

void enviar_a_suscriptor(t_mensaje*, t_suscriptor*);
void enviar_a_suscriptores(t_mensaje*, t_list*);
#endif /* CONEXIONES_H_ */
