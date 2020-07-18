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
t_entrenador* entrenador_create(t_posicion* posicion, t_list* pokemon_obtenidos,
		t_list* objetivos, u_int32_t indice, float estimacion_inicial, char identificador) {

	t_entrenador* entrenador = malloc(sizeof(t_entrenador));

	entrenador->posicion = posicion;
	entrenador->pokemon_obtenidos = pokemon_obtenidos;
	entrenador->objetivos = objetivos;
	entrenador->objetivos_faltantes = NULL; //get_objetivos_faltantes(entrenador);
	get_objetivos_faltantes(entrenador);
	entrenador->estado = NEW;
	entrenador->hilo = 0;
	entrenador->indice = indice;
	entrenador->id_caught = 0;
	entrenador->resultado_caught = 0;
	entrenador->puede_pasar_a_ready = true;
	entrenador->capturas_disponibles = list_size(entrenador->objetivos_faltantes);
	//printf("capturas disponibles: %d\n",entrenador->capturas_disponibles); Me dijo Juan
	entrenador->pokemon_inservibles = list_create();
	entrenador->rafaga = 0;
	entrenador->ciclos_cpu = 0;
	entrenador->identificador = identificador;

	// Agrego cosas para SJF sin desalojo (para discutirlas)
	entrenador->estimacion = estimacion_inicial;
	entrenador->estimacion_restante = estimacion_inicial;
	entrenador->rafaga_anterior = 0;

	entrenador->cantidad_apariciones_deadlock = 0;

	// NOTA: La rafaga inicial la puse inicialmente como 0.
	// En el enunciado no hay nada especificado para rafagas anteriores al
	// inicio del problema. PREGUNTAR

	return entrenador;

}

/*
 * @NAME: list_elem
 * @DESC: Dados un elemento (tipo string) y una lista, me dice si ese
 * 	      elemento se encuentra en la lista.
 */
bool list_elem(char* elemento, t_list* lista) {
	bool encontrado = false;
	for (int i = 0; i < list_size(lista) && !encontrado; i++) {
		char* pokemon = list_get(lista, i);
		encontrado = string_equals_ignore_case(pokemon, elemento);
	}
	return encontrado;
}

bool le_sirve(t_entrenador* entrenador, char* pokemon) {
	return list_elem(pokemon, entrenador->objetivos_faltantes);
}

void cambiar_condicion_ready(t_entrenador* entrenador) {
	entrenador->puede_pasar_a_ready = !entrenador->puede_pasar_a_ready;
}

void atrapar(t_entrenador* entrenador, t_appeared_pokemon* appeared_pokemon) {
	if (!le_sirve(entrenador, appeared_pokemon->pokemon)) list_add(entrenador->pokemon_inservibles, appeared_pokemon->pokemon);
	else list_add(entrenador->pokemon_obtenidos, appeared_pokemon->pokemon);
	decrementar_capturas_disponibles(entrenador);
}

void intercambiar(t_entrenador* entrenador, char* objetivo, char* inservible) {
	bool _es_el_inservible(void* parametro) {
		char* auxiliar = parametro;
		return string_equals_ignore_case(auxiliar, inservible);
	}
	if (!le_sirve(entrenador, objetivo)) list_add(entrenador->pokemon_inservibles, objetivo);
	else list_add(entrenador->pokemon_obtenidos, objetivo);

	list_remove_by_condition(entrenador->pokemon_inservibles, _es_el_inservible);
	get_objetivos_faltantes(entrenador);
}

/*
 * @NAME: cambiar_estado
 * @DESC: Dado un entrenador y un estado pasados por parametro,
 * 		  cambia el estado del entrenador.
 */
void cambiar_estado(t_entrenador* entrenador, t_estado estado) {
	entrenador->estado = estado;
}

/*
 * @NAME: puede_ser_planificado
 * @DESC: Dado un entrenador, devuelve si este puede pasar o no
 *        a estado READY.
 */
bool puede_ser_planificado(void* parametro) {
	t_entrenador* entrenador = parametro;
	return (entrenador->puede_pasar_a_ready);
	//return ((entrenador->estado == NEW) || (entrenador->estado == BLOCK));
}

/*
 * @NAME: remover_estado_repetido
 * @DESC: Dada una lista y el nombre de un pokemon,
 * 		  remueve de la lista la primer aparicion repetida de ese nombre.
 * 		  // revisar
 */
void remover_elemento_repetido(t_list* lista, char* un_pokemon) {
	bool encontrado = false;
	for (int i = 0; i < list_size(lista) && !encontrado; i++) {
		char* otro_pokemon = list_get(lista, i);
		encontrado = string_equals_ignore_case(un_pokemon, otro_pokemon);
		if (encontrado) list_remove(lista, i);
	}
}

/*
 * @NAME: get_objetivos_faltantes
 * @DESC: Dado un entrenador, saca de sus objetivos los pokemon que ya obtuvo.
 * 		  Devuelve la lista de objetivos modificada.
 */
t_list* get_objetivos_faltantes(t_entrenador* entrenador) {
	t_list* objetivos_faltantes = list_create();
	list_add_all(objetivos_faltantes, entrenador->objetivos);
	for (int i = 0; i < list_size(entrenador->pokemon_obtenidos); i++) {
		char* pokemon = list_get(entrenador->pokemon_obtenidos, i);
		remover_elemento_repetido(objetivos_faltantes, pokemon);
	}

	if(entrenador->objetivos_faltantes != NULL) list_destroy(entrenador->objetivos_faltantes);
	entrenador->objetivos_faltantes = objetivos_faltantes;

	return objetivos_faltantes;
}

/*
 * @NAME: get_objetivos_faltantes
 * @DESC: Dado un entrenador, devuelve sus objetivos.
 */
t_list* get_objetivos(t_entrenador* entrenador) {

	return entrenador->objetivos;

}

/*
 * @NAME: get_objetivos_faltantes
 * @DESC: Destruye una estructura t_entrenador.
 */
void entrenador_destroy(t_entrenador* entrenador) {

	free(entrenador->posicion);

	for (int i = 0; i < list_size(entrenador->pokemon_obtenidos); i++) {
		char* pokemon = list_get(entrenador->pokemon_obtenidos, i);
		free(pokemon);
	}

	list_destroy(entrenador->pokemon_inservibles);
	list_destroy(entrenador->pokemon_obtenidos);

	/*for (int i = 0; i < list_size(entrenador->objetivos); i++) {
		char* pokemon = list_get(entrenador->objetivos, i);
		free(pokemon);
	}*/

	list_destroy(entrenador->objetivos);
	list_destroy(entrenador->objetivos_faltantes);

	pthread_join(entrenador->hilo, NULL); // Esto con cancel da leak

	free(entrenador);

}

/*
 * @NAME: puede_seguir_atrapando
 * @DESC: Verifica si un entrenador tiene capturas disponibles.
 */
bool puede_seguir_atrapando(t_entrenador* entrenador) {
	return entrenador->capturas_disponibles > 0;
}

/*
 * @NAME: decrementar_capturas_disponibles
 * @DESC: Dado un entrenador se decrementa en uno sus capturas disponibles.
 */
void decrementar_capturas_disponibles(t_entrenador* entrenador) {
	entrenador->capturas_disponibles -= 1;
}

/*
 * @NAME: set_hilo
 * @DESC: Settea un hilo pasado por parámetro en un entrenador.
 */
void set_hilo(t_entrenador* entrenador, pthread_t hilo) {
	entrenador->hilo = hilo;
}

void set_rafaga_anterior(t_entrenador* entrenador, u_int32_t rafaga_anterior) {
	entrenador->rafaga_anterior = rafaga_anterior;
}

void set_estimacion(t_entrenador* entrenador, float estimacion) {
	entrenador->estimacion = estimacion;
}

bool cumplio_su_objetivo(void* parametro) {
	t_entrenador* entrenador = parametro;
	return list_is_empty(entrenador->objetivos_faltantes);
}

bool no_cumplio_su_objetivo(void* parametro) {
	return !cumplio_su_objetivo(parametro);
}

