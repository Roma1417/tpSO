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
#include <semaphore.h>
#include <commons/collections/queue.h>
#include "entrenador.h"
#include "utils.h"
#include "auxiliar.h"
#include "planificado.h"

u_int32_t id_team;
t_config_team* config_team;
pthread_t hilo_appeared;
pthread_t hilo_localized;
pthread_t hilo_caught;

typedef enum{
	ATRAPAR=1,
	INTERCAMBIAR=2,
	ENVIAR_MENSAJE=3
}tipo_operacion;

void planificar_entrenadores();
void enreadyar_al_mas_cercano(t_list* entrenadores,t_appeared_pokemon* appeared_pokemon);
t_config* leer_config (void);
t_log* iniciar_logger (char* path);
t_list* get_objetivo_global ();
t_config_team* construir_config_team(t_config* config);
void enviar_mensajes_get_pokemon(); // (int conexion);
void* enviar_get_pokemon(void* pokemon);
sem_t* inicializar_vector_de_semaforos(u_int32_t longitud);
void enviar_catch_pokemon(t_entrenador* entrenador, t_appeared_pokemon* pokemon);
void actualizar_objetivo_global();
t_list* obtener_especies();
int ciclos_necesarios(t_entrenador* entrenador, tipo_operacion operacion);

#endif /* TEAM_H_ */
