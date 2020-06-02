/*
 * planificado.h
 *
 *  Created on: 2 jun. 2020
 *      Author: utnso
 */

#ifndef PLANIFICADO_H_
#define PLANIFICADO_H_

#include "entrenador.h"
#include "appeared_pokemon.h"

typedef struct{
	t_entrenador* entrenador;
	t_appeared_pokemon* pokemon;
}t_planificado;

void planificado_destroy(t_planificado* planificado);
t_planificado* planificado_create(t_entrenador* entrenador, t_appeared_pokemon* pokemon);

#endif /* PLANIFICADO_H_ */
