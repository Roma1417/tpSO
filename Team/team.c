/*
 * team.c
 *
 *  Created on: 28 abr. 2020
 *      Author: utnso
 */
#include "team.h"

int main (void) {

	//t_list* entrenadores;
	//t_log* logger = iniciar_logger();
	t_config* config = leer_config();
	t_config_team* config_team = construir_config_team(config);

	//t_lista_pokemon* objetivo_global = get_objetivo_global();



}


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
	config_team -> pokemon_entrenadores = (char**) config_get_array_value(config, "POKEMON_ENTRENADORES");
	//config_team -> objetivos_entrenadores = (t_lista_pokemon*) config_get_array_value(config, "OBJETIVOS_ENTRENADORES");

	t_list* lista = convertir_string_a_lista_de_listas(config_team -> pokemon_entrenadores);

	t_list* primer_lista = list_get(lista, 0);
	char* primer_cadena = list_get(primer_lista,0);

	printf("primer_cadena: %s\n", primer_cadena);

	config_team -> posiciones_entrenadores = NULL;
	config_team -> pokemon_entrenadores = NULL;
	config_team -> objetivos_entrenadores = NULL;
	config_team -> tiempo_reconexion = config_get_int_value(config, "TIEMPO_RECONEXION");
	config_team -> retardo_ciclo_cpu = config_get_int_value(config, "RETARDO_CICLO_CPU");
	config_team -> algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	config_team -> ip_broker = config_get_string_value(config, "IP_BROKER");
	config_team -> estimacion_inicial = config_get_int_value(config, "ESTIMACION_INICIAL");
	config_team -> puerto_broker = config_get_int_value(config, "PUERTO_BROKER");
	config_team -> log_file = config_get_string_value(config, "LOG_FILE");

	return config_team;

}

/*t_list* get_objetivo_global (t_list* entrenadores) {

	return list_map(entrenadores, get_objetivos);

}*/
