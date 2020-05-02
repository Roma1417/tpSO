/*
 * team.h
 *
 *  Created on: 28 abr. 2020
 *      Author: utnso
 */

#ifndef TEAM_H_
#define TEAM_H_

#include "entrenador.h"
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<commons/string.h>

/*typedef struct {

	int posicion_x;
	int posicion_y;

} t_posicion;*/

typedef struct {

	int posicion_x;
	int posicion_y;
	int* sgte;

} t_lista_posicion_entrenador;

typedef struct {

	char* pokemon;
	int* sgte;

}t_lista_pokemon;

typedef struct {

	t_lista_pokemon* pokemons;
	int* sgte;

}t_lista_lista_pokemon;

typedef struct {

	//t_lista_posicion_entrenador* posiciones_entrenadores;
	t_list* posiciones_entrenadores;
	t_list* pokemon_entrenadores;
	t_list* objetivos_entrenadores;
	int tiempo_reconexion;
	int retardo_ciclo_cpu;
	char* algoritmo_planificacion;
	//int quantum; por ahora tiene que ser FIFO asi que no hay quantum
	char* ip_broker;
	int estimacion_inicial;
	int puerto_broker;
	char* log_file;

}t_config_team;

t_config* leer_config (void);
t_log* iniciar_logger (void);
t_list* get_objetivo_global ();
t_config_team* construir_config_team(t_config* config);

#endif /* TEAM_H_ */
