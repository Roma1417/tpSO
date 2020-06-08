/*
 * entrenador.h
 *
 *  Created on: 1 may. 2020
 *      Author: utnso
 */

#ifndef COLLECTIONS_H_
#define COLLECTIONS_H_

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


typedef enum{
	NEW_POKEMON = 1,
	APPEARED_POKEMON = 2,
	CATCH_POKEMON = 3,
	CAUGHT_POKEMON = 4,
	GET_POKEMON = 5,
	LOCALIZED_POKEMON = 6,
	SUSCRIPTOR = 7,
	CONFIRMAR = 8,
}tipo_mensaje;


typedef struct{
	tipo_mensaje id;
	t_list* mensajes;
	t_list* suscriptores;
}t_cola_mensajes;


typedef struct{
	u_int32_t id;
	int numero_socket;
}t_suscriptor;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	u_int32_t id_mensaje;
	tipo_mensaje tipo_mensaje;
	t_buffer* buffer;
} t_paquete;

typedef struct{
	t_paquete* paquete;
	t_list* suscriptores_enviados;
	t_list* suscriptores_confirmados;
}t_mensaje;

t_cola_mensajes* colas_mensajes[6];
u_int32_t generador_id_suscriptor[6];
u_int32_t generador_id_mensaje;
int mutex_id_suscriptor;
int mutex_id_mensaje;



t_cola_mensajes* crear_cola_mensajes(tipo_mensaje);
void destruir_cola_mensajes(t_cola_mensajes*);
void* get_cola_mensajes(tipo_mensaje);

t_suscriptor* crear_suscriptor(u_int32_t, int32_t);
t_suscriptor* generar_suscriptor(u_int32_t, t_cola_mensajes*);
t_suscriptor* buscar_suscriptor(t_list*, u_int32_t);
u_int32_t generar_id_suscriptor(tipo_mensaje);

t_mensaje* crear_mensaje(t_paquete*);
void destruir_mensaje(t_mensaje*);
t_mensaje* buscar_mensaje(t_list*, u_int32_t);
u_int32_t generar_id_mensaje();

t_paquete* crear_paquete(u_int32_t, tipo_mensaje, t_buffer*);
void destruir_paquete(t_paquete*);

t_buffer* crear_buffer(u_int32_t, void*);
void destruir_buffer(t_buffer*);

#endif /* COLLECTIONS_H_ */
