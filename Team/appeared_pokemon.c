/*
 * appeared_pokemon.c
 *
 *  Created on: 13 may. 2020
 *      Author: utnso
 */

#include "appeared_pokemon.h"

/*
 * @NAME: appeared_pokemon_create
 * @DESC: Crea y devuelve un puntero a una estructura t_appeared_pokemon.
 */
t_appeared_pokemon* appeared_pokemon_create() {
	t_appeared_pokemon* appeared_pokemon = malloc(sizeof(t_appeared_pokemon));

	appeared_pokemon->pokemon = NULL;
	appeared_pokemon->size_pokemon = 0;
	appeared_pokemon->posicion = NULL;

	return appeared_pokemon;
}

/*
 * @NAME: appeared_pokemon_destroy
 * @DESC: Destruye una estructura t_appeared_pokemon.
 */
void appeared_pokemon_destroy(t_appeared_pokemon* appeared_pokemon) {

	free(appeared_pokemon->pokemon);
	free(appeared_pokemon->posicion);
	free(appeared_pokemon);

}

/*
 * @NAME: cambiar_size_pokemon
 * @DESC: Dado un appeared_pokemon y un tamanio pasados por parametro,
 * 		  cambia el size del appeared_pokemon.
 */
void cambiar_size_pokemon(t_appeared_pokemon* appeared_pokemon, u_int32_t size_pokemon) {

	appeared_pokemon->size_pokemon = size_pokemon;

}

/*
 * @NAME: cambiar_posicion
 * @DESC: Dado un appeared_pokemon y una posicion pasados por parametro,
 * 		  cambia la posicion del appeared_pokemon.
 */
void cambiar_posicion(t_appeared_pokemon* appeared_pokemon, t_posicion* posicion) {

	appeared_pokemon->posicion = posicion;

}

/*
 * @NAME: cambiar_nombre_pokemon
 * @DESC: Dado un appeared_pokemon y un nombre pasados por parametro,
 * 		  cambia el nombre del appeared_pokemon.
 */
void cambiar_nombre_pokemon(t_appeared_pokemon* appeared_pokemon, char* pokemon) {

	appeared_pokemon->pokemon = pokemon;

}
