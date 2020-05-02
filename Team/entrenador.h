/*
 * entrenador.h
 *
 *  Created on: 1 may. 2020
 *      Author: utnso
 */

#ifndef ENTRENADOR_H_
#define ENTRENADOR_H_

#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include "team.h"


typedef struct {

	int posicion_x;
	int posicion_y;

} t_posicion;

typedef struct {

	t_posicion* posicion;
	t_list* pokemon_obtenidos;
	t_list* objetivos;

	// ID
	// Hilo
	// Estado

} t_entrenador;

t_entrenador* entrenador_create(t_posicion* posicion, t_list* pokemon_obtenidos, t_list* objetivos);
t_list* get_objetivos();

#endif /* ENTRENADOR_H_ */
