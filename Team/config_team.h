/*
 * config_team.h
 *
 *  Created on: 20 jun. 2020
 *      Author: utnso
 */

#ifndef CONFIG_TEAM_H_
#define CONFIG_TEAM_H_

#include <commons/collections/list.h>

typedef struct {

	//t_lista_posicion_entrenador* posiciones_entrenadores;
	t_list* posiciones_entrenadores;
	t_list* pokemon_entrenadores;
	t_list* objetivos_entrenadores;
	int tiempo_reconexion;
	int retardo_ciclo_cpu;
	char* algoritmo_planificacion;
	float alpha; // Agrego el alpha que esta especificado en el enunciado
	int quantum;
	char* ip_broker;
	int estimacion_inicial;
	char* puerto_broker;
	char* log_file;

}t_config_team;

#endif /* CONFIG_TEAM_H_ */
