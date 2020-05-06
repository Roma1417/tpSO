/*
 * team.c
 *
 *  Created on: 28 abr. 2020
 *      Author: utnso
 */
#include "team.h"

t_log* iniciar_logger(void) {

	t_log* logger = log_create("team.log","team", true, LOG_LEVEL_INFO);
	if(logger == NULL) {
		 printf("No pude crear el logger\n");
		 exit(1);
	}
	return logger;

}

t_config* leer_config(void) {

	t_config* config = config_create("./team.config");

	return config;

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
				posicion -> posicion_x = atoi(cadena);
				free(cadena);
				cadena = string_new();
			}
			if (j == string_length(cadenas[i])) {
				posicion -> posicion_y = atoi(cadena);
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

t_config_team* construir_config_team(t_config* config){

	t_config_team* config_team = malloc(sizeof(t_config_team));

	config_team -> puerto_broker = config_get_int_value(config, "PUERTO_BROKER");

	// A revisar el config_get_array_value
	char** objetivos = config_get_array_value(config, "OBJETIVOS_ENTRENADORES");
	t_list* lista_objetivos = convertir_string_a_lista_de_listas(objetivos);
	config_team -> objetivos_entrenadores = lista_objetivos;

	char** pokemons = config_get_array_value(config, "POKEMON_ENTRENADORES");
	t_list* lista_pokemons = convertir_string_a_lista_de_listas(pokemons);
	config_team -> pokemon_entrenadores = lista_pokemons;

	char** posiciones = config_get_array_value(config, "POSICIONES_ENTRENADORES");
	t_list* lista_posiciones = convertir_string_a_lista_de_posiciones(posiciones);
	config_team -> posiciones_entrenadores = lista_posiciones;

	config_team -> tiempo_reconexion = config_get_int_value(config, "TIEMPO_RECONEXION");
	config_team -> retardo_ciclo_cpu = config_get_int_value(config, "RETARDO_CICLO_CPU");
	config_team -> algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	config_team -> ip_broker = config_get_string_value(config, "IP_BROKER");
	config_team -> estimacion_inicial = config_get_int_value(config, "ESTIMACION_INICIAL");
	config_team -> log_file = config_get_string_value(config, "LOG_FILE");

	return config_team;

}

//Codigo de prueba
void* print_message_function(){

	printf("Soy un hilo\n");

	return EXIT_SUCCESS;
}

t_list* crear_entrenadores(t_config_team* config_team){

	t_list* entrenadores = list_create();

	t_list* objetivos_entrenadores = config_team -> objetivos_entrenadores;
	t_list* pokemon_entrenadores = config_team -> pokemon_entrenadores;
	t_list* posiciones_entrenadores = config_team -> posiciones_entrenadores;

	for(int i = 0; i < list_size(objetivos_entrenadores); i++){
		t_list* objetivo = list_get(objetivos_entrenadores,i);
		t_list* pokemon_obtenidos = list_get(pokemon_entrenadores,i);
		t_posicion* posicion = list_get(posiciones_entrenadores, i);

		// Codigo en prueba
		pthread_t hilo;
	    int iret = pthread_create(&hilo, NULL, print_message_function, NULL);
	    if (iret) exit(1);

		t_entrenador* entrenador = entrenador_create(posicion, pokemon_obtenidos, objetivo, hilo);
		list_add(entrenadores, entrenador);
	}

	return entrenadores;

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

// Codigo a revisar
t_list* get_objetivo_global (t_list* entrenadores) {

	return list_map(entrenadores, (void*) get_objetivos);

}

void destruir_config_team(t_config_team* config_team){
	list_destroy(config_team->objetivos_entrenadores);
	list_destroy(config_team->pokemon_entrenadores);
	list_destroy(config_team->posiciones_entrenadores);

	free(config_team -> algoritmo_planificacion);
	free(config_team -> ip_broker);
	free(config_team -> log_file);
}

void liberar_estructuras(t_config_team* config_team, t_list* entrenadores){

	for(int i = 0; i < list_size(entrenadores); i++){
		t_entrenador* entrenador = list_get(entrenadores,i);
		destruir_entrenador(entrenador);
	}

	destruir_config_team(config_team);

}

void terminar_programa(t_log* logger, t_config* config){
	if (logger != NULL) log_destroy(logger);
	if (config != NULL) config_destroy(config);
}

int main (void) {

	t_list* entrenadores;
	t_log* logger = iniciar_logger();
	t_config* config = leer_config();
	t_config_team* config_team = construir_config_team(config);

	entrenadores = crear_entrenadores(config_team);

	//t_list* objetivo_global = get_objetivo_global(entrenadores);

	//t_list* objetivo_global_sin_repetidos = eliminar_repetidos(objetivo_global);

	liberar_estructuras(config_team, entrenadores);

	terminar_programa(logger, config);

	//iniciar_servidor();

	return 0;

}
