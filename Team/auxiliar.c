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
	u_int32_t suma_de_potencias = pow(diferencia_en_x, 2) + pow(diferencia_en_y, 2);
	return sqrt(suma_de_potencias);
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

// Separador

void agregar_a_la_lista(t_list* lista_pokemon, char* pokemon){
	if (pokemon != NULL) list_add(lista_pokemon, pokemon);
}


t_list* pasar_a_lista_de_pokemon(t_config* config, char* cadena) {
  char** read_array = config_get_array_value(config, cadena);

  t_list* pokemon = list_create();
  t_list* sublista;

  void _a_la_lista(char *poke) {
    if (poke != NULL) {
      list_add(sublista, poke);
    }
  }

  void _dividir(char *string) {
	sublista = list_create();
    if(string != NULL) {
      char** pokes = string_split(string, "|");
      string_iterate_lines(pokes, _a_la_lista);
      free(pokes);
    } else exit(1);
    list_add(pokemon,sublista);
  }
  string_iterate_lines(read_array, _dividir);

  string_iterate_lines(read_array, (void*) free);

  free(read_array);
  return pokemon;
}

t_list* pasar_a_lista_de_posiciones(t_config* config, char* cadena) {
  char** read_array = config_get_array_value(config, cadena);

  t_list* posiciones = list_create();
  t_posicion* posicion;

  void _dividir(char *string) {
    if(string != NULL) {
      char** punto = string_split(string, "|");
      u_int32_t x = atoi(punto[0]);
      u_int32_t y = atoi(punto[1]);
      posicion = posicion_create(x,y);
      string_iterate_lines(punto, (void*) free);
      free(punto);
    } else exit(1);
    list_add(posiciones,posicion);
  }

  string_iterate_lines(read_array, _dividir);

  string_iterate_lines(read_array, (void*) free);

  free(read_array);



  return posiciones;
}

