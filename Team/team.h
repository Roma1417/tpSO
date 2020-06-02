/*
 * team.h
 *
 *  Created on: 28 abr. 2020
 *      Author: utnso
 */

#ifndef TEAM_H_
#define TEAM_H_

#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <commons/collections/queue.h>
#include "entrenador.h"
#include "utils.h"
#include "auxiliar.h"
#include "planificado.h"

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
	char* puerto_broker;
	char* log_file;

}t_config_team;

u_int32_t id_team;
t_config_team* config_team;

void planificar_entrenadores();
void enreadyar_al_mas_cercano(t_list* entrenadores,t_appeared_pokemon* appeared_pokemon);
t_config* leer_config (void);
t_log* iniciar_logger (void);
t_list* get_objetivo_global ();
t_config_team* construir_config_team(t_config* config);
void enviar_mensajes_get_pokemon(); // (int conexion);

#endif /* TEAM_H_ */
