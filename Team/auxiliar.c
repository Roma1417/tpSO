/*
 * auxiliar.c
 *
 *  Created on: 9 may. 2020
 *      Author: utnso
 */
#include "auxiliar.h"

u_int32_t distancia(t_entrenador* entrenador, t_appeared_pokemon* appeared_pokemon){
	u_int32_t diferencia_en_x = entrenador->posicion->x - appeared_pokemon->posicion->x;
	//printf("diferencia en x: %d, entrenador x: %d, pokemon x: %d\n", diferencia_en_x, entrenador->posicion->x, appeared_pokemon->posicion->x);
	u_int32_t diferencia_en_y = entrenador->posicion->y - appeared_pokemon->posicion->y;
	//printf("diferencia en y: %d, entrenador y: %d, pokemon y: %d\n", diferencia_en_y, entrenador->posicion->y, appeared_pokemon->posicion->y);
	//printf("abs x: %d, abs y: %d\n",abs(diferencia_en_x), abs(diferencia_en_y));
	return abs(diferencia_en_x) + abs(diferencia_en_y);
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

