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
void cambiar_estado(t_entrenador* entrenador, t_estado estado){
	entrenador->estado = estado;
}

bool puede_pasar_a_ready(void* parametro){
	t_entrenador* entrenador = parametro;
	return ((entrenador->estado == NEW) || (entrenador->estado == BLOCK));
}

void remover_elemento_repetido(t_list* lista, char* un_pokemon){
	bool encontrado = false;
	for (int i = 0; i < list_size(lista) && !encontrado; i++){
		char* otro_pokemon = list_get(lista, i);
		encontrado = string_equals_ignore_case(un_pokemon, otro_pokemon);
		if (encontrado){
			char* removido = list_remove(lista, i);
			free(removido);
		}
	}
}

t_list* get_objetivos_faltantes(t_entrenador* entrenador){
	for (int i = 0; i < list_size(entrenador->pokemon_obtenidos); i++){
		char* pokemon = list_get(entrenador->pokemon_obtenidos, i);
		remover_elemento_repetido(entrenador->objetivos, pokemon);
	}

	return entrenador->objetivos;
}

t_list* get_objetivos(t_entrenador* entrenador){

	return entrenador->objetivos;

}

void entrenador_destroy(t_entrenador* entrenador){

	free(entrenador->posicion);

	for(int i = 0; i < list_size(entrenador->pokemon_obtenidos); i++){
		char* pokemon = list_get(entrenador->pokemon_obtenidos, i);
		free(pokemon);
	}

	list_destroy(entrenador->pokemon_obtenidos);

	for(int i = 0; i < list_size(entrenador->objetivos); i++){
		char* pokemon = list_get(entrenador->objetivos, i);
		free(pokemon);
	}

	list_destroy(entrenador->objetivos);

	pthread_join(entrenador->hilo, NULL); // Esto con cancel da leak

	free(entrenador);

}
