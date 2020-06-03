/*
 * planificado.c
 *
 *  Created on: 2 jun. 2020
 *      Author: utnso
 */

#include "planificado.h"

/*
 * @NAME: planificado_create
 * @DESC: Dados un entrenador y un appeared_pokemon, crea y devuelve
 * 		  una estructura t_planificado.
 */
t_planificado* planificado_create(t_entrenador* entrenador, t_appeared_pokemon* pokemon){
	t_planificado* planificado = malloc(sizeof(t_planificado));
	planificado->entrenador = entrenador;
	planificado->pokemon = pokemon;
	return planificado;
}

/*
 * @NAME: planificado_destroy
 * @DESC: Destruye una estructura t_planificado.
 */
void planificado_destroy(t_planificado* planificado){
	entrenador_destroy(planificado->entrenador);
	appeared_pokemon_destroy(planificado->pokemon);
	free(planificado);
}
