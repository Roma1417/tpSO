/*
 * entrenador.h
 *
 *  Created on: 1 may. 2020
 *      Author: utnso
 */

#ifndef ENTRENADOR_H_
#define ENTRENADOR_H_

#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "team.h"
#include "posicion.h"

typedef struct {

	u_int32_t posicion_x;
	u_int32_t posicion_y;

} t_posicion;

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
	pthread_t* hilo;

	// ID
	// Hilo

} t_entrenador;

t_entrenador* entrenador_create(t_posicion* posicion, t_list* pokemon_obtenidos, t_list* objetivos, pthread_t hilo);
t_list* get_objetivos();
void destruir_entrenador(t_entrenador* entrenador);

#endif /* ENTRENADOR_H_ */
