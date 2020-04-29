/*
 * team.c
 *
 *  Created on: 28 abr. 2020
 *      Author: utnso
 */
#include "team.h"

int main (void) {

	t_log* logger = iniciar_logger();
	t_config_team* config = leer_config();

	t_lista_pokemon* objetivo_global = get_objetivo_global();

}


t_log* iniciar_logger(void) {

	t_log* logger = log_create("team.log","team", true, LOG_LEVEL_INFO);
	if(logger == NULL) {
		 printf("No pude crear el logger\n");
		 exit(1);
	}
	return logger;

}

t_config_team* leer_config(void) {

	t_config_team* config_team = malloc(sizeof(t_config_team));
	t_config* config = config_create("./team.config");

	config_team -> pokemon_entrenadores = (t_lista_posicion_entrenador*) config_get_array_value(config, "POSICIONES_ENTRENADORES");
	config_team -> pokemon_entrenadores = (t_lista_pokemon*) config_get_array_value(config, "POKEMON_ENTRENADORES");
	config_team -> objetivos_entrenadores = (t_lista_pokemon*) config_get_array_value(config, "OBJETIVOS_ENTRENADORES");
	config_team -> tiempo_reconexion = config_get_int_value(config, "TIEMPO_RECONEXION");
	config_team -> retardo_ciclo_cpu = config_get_int_value(config, "RETARDO_CICLO_CPU");
	config_team -> algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	config_team -> ip_broker = config_get_string_value(config, "IP_BROKER");
	config_team -> estimacion_inicial = config_get_int_value(config, "ESTIMACION_INICIAL");
	config_team -> puerto_broker = config_get_int_value(config, "PUERTO_BROKER");
	config_team -> log_file = config_get_string_value(config, "LOG_FILE");


	return config;

}

t_lista_pokemon* get_objetivo_global () {

	t_lista_pokemon* objetivo_global;

	while() {
		//recorrer lista entrenadores
	}

	return objetivo_global;

}
