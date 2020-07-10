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
	tipo_mensaje tipo_mensaje;
	u_int32_t id_mensaje;
	uint32_t id_correlativo;
	t_buffer* buffer;
} t_paquete;

typedef struct{
	void* base;
	uint32_t tamanio;
	t_list* particiones;
}t_memoria;

typedef struct{
	uint32_t lru;
	uint32_t cola_mensajes;
	uint32_t id_mensaje;
	uint32_t id_correlativo;
	t_list* suscriptores_enviados;
	t_list* suscriptores_confirmados;
}t_atributos_particion;

typedef struct{
	void* base;
	uint32_t tamanio;
	bool ocupada;
	t_atributos_particion* atributos;
}t_particion;


t_list* lista_suscriptores[6];
u_int32_t generador_id_suscriptor[6];
u_int32_t generador_id_mensaje;

t_memoria* crear_memoria(uint32_t);

t_particion* crear_particion(void*, uint32_t, bool, t_atributos_particion*);
t_particion* buscar_particion(t_memoria*, uint32_t);
void destruir_particion(t_particion* particion);

t_atributos_particion* crear_atributos_particion(uint32_t, uint32_t, uint32_t, uint32_t);
void destruir_atributos_particion(t_atributos_particion* atributos_particion);

t_suscriptor* crear_suscriptor(u_int32_t, int32_t);
t_suscriptor* generar_suscriptor(u_int32_t, uint32_t);
t_suscriptor* buscar_suscriptor(t_list*, u_int32_t);
u_int32_t generar_id_suscriptor(tipo_mensaje);
t_list* obtener_lista_suscriptores(uint32_t);
void destruir_suscriptor(void* suscriptor);

u_int32_t generar_id_mensaje();

t_paquete* crear_paquete(u_int32_t, uint32_t, tipo_mensaje, t_buffer*);
t_paquete* generar_paquete(t_particion* particion);
void destruir_paquete(t_paquete*);

t_buffer* crear_buffer(u_int32_t, void*);
void destruir_buffer(t_buffer*);

#endif /* COLLECTIONS_H_ */
