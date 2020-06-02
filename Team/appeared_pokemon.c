/*
 * appeared_pokemon.c
 *
 *  Created on: 13 may. 2020
 *      Author: utnso
 */

#include "appeared_pokemon.h"

t_appeared_pokemon* appeared_pokemon_create(){
	t_appeared_pokemon* appeared_pokemon = malloc(sizeof(t_appeared_pokemon));

	appeared_pokemon->pokemon = NULL;
	appeared_pokemon->size_pokemon = 0;
	appeared_pokemon->posicion = NULL;

	return appeared_pokemon;
}

void appeared_pokemon_destroy(t_appeared_pokemon* appeared_pokemon){

	free(appeared_pokemon->pokemon);
	free(appeared_pokemon->posicion);
	free(appeared_pokemon);

}

void cambiar_size_pokemon(t_appeared_pokemon* appeared_pokemon, u_int32_t size_pokemon){

	appeared_pokemon->size_pokemon = size_pokemon;

}

void cambiar_posicion(t_appeared_pokemon* appeared_pokemon, t_posicion* posicion){

	appeared_pokemon->posicion = posicion;

}

void cambiar_nombre_pokemon(t_appeared_pokemon* appeared_pokemon, char* pokemon){

	appeared_pokemon->pokemon = pokemon;

}
