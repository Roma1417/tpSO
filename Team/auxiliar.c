/*
 * auxiliar.c
 *
 *  Created on: 9 may. 2020
 *      Author: utnso
 */
#include "auxiliar.h"

u_int32_t distancia(t_entrenador* entrenador, t_appeared_pokemon* appeared_pokemon){
	u_int32_t diferencia_en_x = entrenador->posicion->x - appeared_pokemon->posicion->x;
	u_int32_t diferencia_en_y = entrenador->posicion->y - appeared_pokemon->posicion->y;
	return sqrt(pow(diferencia_en_x, 2) + pow(diferencia_en_y, 2));
}

t_list* convertir_string_a_lista_de_listas(char** cadenas){
	t_list* listas = list_create();
	char* cadena;
	int longitud = sizeof(cadenas) - 1;
	for(int i = 0; i < longitud; i++){
		t_list* sublista = list_create();
		for(int j = 0; j < string_length(cadenas[i])+1; j++){
			if (j == 0){
				cadena = string_new();
			}
			if (cadenas[i][j] == '|'){
				list_add(sublista, cadena);
				cadena = string_new();
			}
			if (j == string_length(cadenas[i])) {
				list_add(sublista, cadena);
			}
			if(cadenas[i][j] != '|') string_append_with_format(&cadena, "%c",cadenas[i][j]);
		}
		list_add(listas,sublista);
	}
	return listas;
}

t_list* convertir_string_a_lista_de_posiciones(char** cadenas){
	t_list* posiciones = list_create();
	t_posicion* posicion;
	char* cadena;
	int longitud = sizeof(cadenas) - 1;
	for(int i = 0; i < longitud; i++){
		for(int j = 0; j < string_length(cadenas[i])+1; j++){
			if (j == 0){
				posicion = malloc(sizeof(t_posicion));
				cadena = string_new();
			}
			if (cadenas[i][j] == '|'){
				posicion -> x = atoi(cadena);
				free(cadena);
				cadena = string_new();
			}
			if (j == string_length(cadenas[i])) {
				posicion -> y = atoi(cadena);
				free(cadena);
				list_add(posiciones, posicion);
			}
			if(cadenas[i][j] != '|') {
				string_append_with_format(&cadena, "%c",cadenas[i][j]);
			}
		}
	}
	return posiciones;
}

t_list* list_flatten(t_list* listas){

	t_list* lista = list_create();

	for(int i = 0; i < list_size(listas); i++){
		t_list* sublista = list_get(listas,i);
		for(int j = 0; j < list_size(sublista); j++){
			char* pokemon = list_get(sublista,j);
			list_add(lista, pokemon);
		}
	}

	return lista;
}

bool list_elem(char* elemento, t_list* lista){
	bool encontrado = false;
	for(int i = 0; i < list_size(lista) && !encontrado; i++){
		char* pokemon = list_get(lista, i);
		encontrado = string_equals_ignore_case(pokemon, elemento);
	}
	return encontrado;
}

t_list* eliminar_repetidos(t_list* objetivo_global){

	t_list* lista_aplanada = list_flatten(objetivo_global);
	t_list* especies_requeridas = list_create();

	for(int i = 0; i < list_size(lista_aplanada); i++){
		char* pokemon = list_get(lista_aplanada, i);
		if(!list_elem(pokemon, especies_requeridas)){
			list_add(especies_requeridas, pokemon);
		}
	}

	list_destroy(lista_aplanada);

	return especies_requeridas;
}
