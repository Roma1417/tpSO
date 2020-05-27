/*
 * entrenador.h
 *
 *  Created on: 1 may. 2020
 *      Author: utnso
 */

#ifndef COLA_MENSAJES_H_
#define COLA_MENSAJES_H_

#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

typedef enum{
	NEW_POKEMON = 1,
	APPEARED_POKEMON = 2,
	CATCH_POKEMON = 3,
	CAUGHT_POKEMON = 4,
	GET_POKEMON = 5,
	LOCALIZED_POKEMON = 6,
	SUSCRIPTOR = 7,
	EXIT = 9,
}tipo_mensaje;


typedef struct{
	tipo_mensaje id;
	t_queue* mensajes;
	t_list* suscriptores;
}t_cola_mensajes;


typedef struct{
	u_int32_t id;
	u_int32_t socket;
}t_suscriptor;


typedef struct{
	u_int32_t id_mensaje;
	u_int32_t size;
	void* stream;
	t_list* suscriptores_enviados;
	t_list* suscriptores_confirmados;
}t_mensaje;

t_cola_mensajes* colas_mensajes[6];



t_cola_mensajes* cola_mensajes_create(tipo_mensaje);
void cola_mensajes_destroy(t_cola_mensajes*);
void set_cola_mensajes(char*);
void* get_cola_mensajes(tipo_mensaje);

t_suscriptor* crear_suscriptor(u_int32_t, u_int32_t);
t_suscriptor* agregar_suscriptor(u_int32_t, t_cola_mensajes*);
t_suscriptor* actualizar_suscriptor(u_int32_t, t_cola_mensajes*, u_int32_t);
t_suscriptor* buscar_suscriptor(t_cola_mensajes*, u_int32_t);
u_int32_t generar_id_suscriptor(tipo_mensaje);
void eliminar_suscriptor(int32_t*, t_cola_mensajes*);

t_mensaje* crear_mensaje(u_int32_t, void*);
void agregar_mensaje(t_mensaje*, t_cola_mensajes*);

#endif /* COLA_MENSAJES_H_ */
