/*
 * entrenador.c
 *
 *  Created on: 1 may. 2020
 *      Author: utnso
 */

#include "entrenador.h"

t_entrenador* entrenador_create(t_posicion* posicion, t_list* pokemon_obtenidos, t_list* objetivos, pthread_t hilo){

	t_entrenador* entrenador = malloc(sizeof(t_entrenador));

	entrenador->posicion = posicion;
	entrenador->pokemon_obtenidos = pokemon_obtenidos;
	entrenador->objetivos = objetivos;
	entrenador->estado = NEW;
	entrenador->hilo = hilo;

	return entrenador;

}

t_list* get_objetivos(t_entrenador* entrenador){

	return entrenador->objetivos;

}

void destruir_entrenador(t_entrenador* entrenador){

	free(entrenador->posicion);

	list_destroy(entrenador->pokemon_obtenidos);
	list_destroy(entrenador->objetivos);

	pthread_cancel(entrenador->hilo);

	free(entrenador);

}
