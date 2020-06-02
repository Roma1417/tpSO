/*
 * appeared_pokemon.h
 *
 *  Created on: 13 may. 2020
 *      Author: utnso
 */

#ifndef APPEARED_POKEMON_H_
#define APPEARED_POKEMON_H_

#include <stdint.h>
#include <stdlib.h>
#include "posicion.h"

typedef struct{

	u_int32_t size_pokemon;
	char* pokemon;
	t_posicion* posicion;

} t_appeared_pokemon;

t_appeared_pokemon* appeared_pokemon_create();
void appeared_pokemon_destroy(t_appeared_pokemon* appeared_pokemon);
void cambiar_size_pokemon(t_appeared_pokemon* appeared_pokemon, u_int32_t size_pokemon);
void cambiar_nombre_pokemon(t_appeared_pokemon* appeared_pokemon, char* pokemon);
void cambiar_posicion(t_appeared_pokemon* appeared_pokemon, t_posicion* posicion);

#endif /* APPEARED_POKEMON_H_ */
