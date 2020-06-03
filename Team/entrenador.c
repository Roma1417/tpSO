/*
 * entrenador.c
 *
 *  Created on: 1 may. 2020
 *      Author: utnso
 */

#include "entrenador.h"

/*
 * @NAME: entrenador_create
 * @DESC: Dados una posicion, una lista con los pokemon obtenidos,
 *        una lista con los objetivos y un hilo,
 *        crea y devuelve un puntero a una estructura t_entrenador.
 */
t_entrenador* entrenador_create(t_posicion* posicion, t_list* pokemon_obtenidos, t_list* objetivos, u_int32_t id){

	t_entrenador* entrenador = malloc(sizeof(t_entrenador));

	entrenador->posicion = posicion;
	entrenador->pokemon_obtenidos = pokemon_obtenidos;
	entrenador->objetivos = objetivos;
	entrenador->estado = NEW;
	entrenador->hilo = NULL;
	entrenador->capturas_disponibles = list_size(objetivos);
	entrenador->id = id;

	return entrenador;

}

/*
 * @NAME: cambiar_estado
 * @DESC: Dado un entrenador y un estado pasados por parametro,
 * 		  cambia el estado del entrenador.
 */
void cambiar_estado(t_entrenador* entrenador, t_estado estado){
	entrenador->estado = estado;
}

/*
 * @NAME: cambiar_estado
 * @DESC: Dado un entrenador, devuelve si este puede pasar o no
 *        a estado READY.
 */
bool puede_pasar_a_ready(void* parametro){
	t_entrenador* entrenador = parametro;
	return ((entrenador->estado == NEW) || (entrenador->estado == BLOCK));
}

/*
 * @NAME: remover_estado_repetido
 * @DESC: Dada una lista y el nombre de un pokemon,
 * 		  remueve de la lista la primer aparicion repetida de ese nombre.
 * 		  // revisar
 */
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

/*
 * @NAME: get_objetivos_faltantes
 * @DESC: Dado un entrenador, saca de sus objetivos los pokemon que ya obtuvo.
 * 		  Devuelve la lista de objetivos modificada.
 */
t_list* get_objetivos_faltantes(t_entrenador* entrenador){
	for (int i = 0; i < list_size(entrenador->pokemon_obtenidos); i++){
		char* pokemon = list_get(entrenador->pokemon_obtenidos, i);
		remover_elemento_repetido(entrenador->objetivos, pokemon);
	}

	return entrenador->objetivos;
}

/*
 * @NAME: get_objetivos_faltantes
 * @DESC: Dado un entrenador, devuelve sus objetivos.
 */
t_list* get_objetivos(t_entrenador* entrenador){

	return entrenador->objetivos;

}

/*
 * @NAME: get_objetivos_faltantes
 * @DESC: Destruye una estructura t_entrenador.
 */
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

/*
 * @NAME: puede_seguir_atrapando
 * @DESC: Verifica si un entrenador tiene capturas disponibles.
 */
bool puede_seguir_atrapando(t_entrenador* entrenador){
	return entrenador->capturas_disponibles > 0;
}

/*
 * @NAME: decrementar_capturas_disponibles
 * @DESC: Dado un entrenador se decrementa en uno sus capturas disponibles.
 */
void decrementar_capturas_disponibles(t_entrenador* entrenador){
	entrenador->capturas_disponibles--;
}

/*
 * @NAME: set_hilo
 * @DESC: Settea un hilo pasado por parámetro en un entrenador.
 */
void set_hilo(t_entrenador* entrenador, pthread_t hilo){
	entrenador->hilo = hilo;
}

