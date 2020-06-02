/*
 * planificado.c
 *
 *  Created on: 2 jun. 2020
 *      Author: utnso
 */

#include "planificado.h"

t_planificado* planificado_create(t_entrenador* entrenador, t_appeared_pokemon* pokemon){
	t_planificado* planificado = malloc(sizeof(t_planificado));
	planificado->entrenador = entrenador;
	planificado->pokemon = pokemon;
	return planificado;
}

void planificado_destroy(t_planificado* planificado){
	entrenador_destroy(planificado->entrenador);
	appeared_pokemon_destroy(planificado->pokemon);
	free(planificado);
}
