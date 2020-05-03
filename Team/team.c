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

//Codigo a revisar
t_list* convertir_string_a_lista_de_listas(char** cadenas){
	t_list* listas = list_create();
	char* cadena;
	int longitud = sizeof(cadenas);
	for(int i = 0; i < longitud - 1; i++){
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

t_config_team* construir_config_team(t_config* config){

	t_config_team* config_team = malloc(sizeof(t_config_team));

	//config_team -> posiciones_entrenadores = (char**) config_get_array_value(config, "POSICIONES_ENTRENADORES");
	//config_team -> pokemon_entrenadores = (char**) config_get_array_value(config, "POKEMON_ENTRENADORES");

	char** objetivos = config_get_array_value(config, "OBJETIVOS_ENTRENADORES");
	t_list* lista_objetivos = convertir_string_a_lista_de_listas(objetivos);
	config_team -> objetivos_entrenadores = lista_objetivos;

	//Codigo de Prueba
	t_list* primer_lista = list_get(lista_objetivos, 0);
	char* primer_cadena = list_get(primer_lista,0);
	printf("primer_cadena1: %s\n", primer_cadena);

	config_team -> posiciones_entrenadores = NULL;
	config_team -> pokemon_entrenadores = NULL;
	config_team -> tiempo_reconexion = config_get_int_value(config, "TIEMPO_RECONEXION");
	config_team -> retardo_ciclo_cpu = config_get_int_value(config, "RETARDO_CICLO_CPU");
	config_team -> algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	config_team -> ip_broker = config_get_string_value(config, "IP_BROKER");
	config_team -> estimacion_inicial = config_get_int_value(config, "ESTIMACION_INICIAL");
	config_team -> puerto_broker = config_get_int_value(config, "PUERTO_BROKER");
	config_team -> log_file = config_get_string_value(config, "LOG_FILE");

	return config_team;

}

// Codigo de prueba y a revisar (el for)
t_list* crear_entrenadores(t_config_team* config_team){

	t_list* entrenadores = list_create();

	t_list* objetivos_entrenadores = config_team->objetivos_entrenadores;

	for(int i = 0; i < list_size(objetivos_entrenadores); i++){
		t_list* objetivo = list_get(objetivos_entrenadores,i);
		t_entrenador* entrenador = entrenador_create(NULL, NULL, objetivo);
		list_add(entrenadores, entrenador);

	}

	t_entrenador* entrenador0 = list_get(entrenadores, 0);

	char* primer_cadena = list_get(entrenador0->objetivos,0);

	printf("primer_cadena2: %s\n", primer_cadena);

	return entrenadores;

}

// Codigo a revisar
t_list* get_objetivo_global (t_list* entrenadores) {

	return list_map(entrenadores, (void*) get_objetivos);

}

void liberar_estructuras(t_config_team* config_team, t_list* entrenadores){

	for(int i = 0; i < list_size(entrenadores); i++){
		t_entrenador* entrenador = list_get(entrenadores,i);
		destruir_entrenador(entrenador);
	}

	free(config_team->objetivos_entrenadores);

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

	t_list* objetivo_global = get_objetivo_global(entrenadores);

	// Codigo de prueba
	t_list* primer_objetivo = list_get(objetivo_global, 0);
	char* primer_cadena = list_get(primer_objetivo,0);
	printf("primer_cadena3: %s\n", primer_cadena);


	//liberar_estructuras(config_team, entrenadores);

	terminar_programa(logger, config);

	return 0;

}
