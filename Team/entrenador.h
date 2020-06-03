/*
 * entrenador.h
 *
 *  Created on: 1 may. 2020
 *      Author: utnso
 */

#ifndef ENTRENADOR_H_
#define ENTRENADOR_H_

#include <stdlib.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "posicion.h"

typedef enum{
	NEW = 1,
	READY = 2,
	EXEC = 3,
	BLOCK = 4,
	EXIT = 5,
} t_estado;

typedef struct {

	t_posicion* posicion;
	t_list* pokemon_obtenidos;
	t_list* objetivos;
	t_estado estado;
	pthread_t hilo;
	u_int32_t capturas_disponibles;
	u_int32_t id;

} t_entrenador;



void cambiar_estado(t_entrenador* entrenador, t_estado estado);
bool puede_pasar_a_ready(void* parametro);
t_entrenador* entrenador_create(t_posicion* posicion, t_list* pokemon_obtenidos, t_list* objetivos, u_int32_t id);
t_list* get_objetivos(t_entrenador* entrenador);
t_list* get_objetivos_faltantes(t_entrenador* entrenador);
void entrenador_destroy(t_entrenador* entrenador);
bool puede_seguir_atrapando(t_entrenador* entrenador);
void decrementar_capturas_disponibles(t_entrenador* entrenador);
void set_hilo(t_entrenador* entrenador, pthread_t hilo);

#endif /* ENTRENADOR_H_ */
