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

#define ATRAPAR 1
#define INTERCAMBIAR 5
#define ENVIAR_MENSAJE 1

u_int32_t id_team;
u_int32_t cantidad_deadlocks;
u_int32_t largo_lista_ready;
sem_t mutex_largo_lista_ready;


t_config* leer_config(void);
t_config_team* construir_config_team(t_config* config);
t_log* iniciar_logger(char* path);
t_list* get_objetivo_global();
t_list* obtener_especies();
t_list* crear_entrenadores(t_config_team* config_team);
sem_t* inicializar_vector_de_semaforos(u_int32_t longitud);
int32_t enviar_catch_pokemon(t_entrenador* entrenador, t_appeared_pokemon* pokemon);
void liberar_estructuras(t_config_team* config_team, t_list* entrenadores, t_queue* cola_ready, t_list* objetivo_global, t_list* especies_requeridas);
void enreadyar_al_mas_cercano(t_list* entrenadores, t_appeared_pokemon* appeared_pokemon);
void enviar_mensajes_get_pokemon(); // (int conexion);
void intercambiar_pokemon_FIFO(t_entrenador* entrenador, t_planificado* planificado);
void intercambiar_pokemon_RR(t_entrenador* entrenador, t_planificado* planificado);
void* enviar_get_pokemon(void* pokemon);
void actualizar_objetivo_global();
void informar_resultados();
void* ejecutar_entrenador_FIFO(void* parametro);
void* ejecutar_entrenador_RR(void* parametro);
void* iniciar_planificador_largo_plazo(void* parametro);
void* iniciar_planificador();
void* iniciar_intercambiador();
void realizar_intercambios_FIFO();
void realizar_intercambios_RR();
void planificar_entrenadores();
void* mantener_servidor();
void* suscribirse(void* cola);
void suscribirse_a_colas();
void destruir_config_team(t_config_team* config_team);
void destruir_appeared_pokemons();
void terminar_programa(t_log* logger, t_config* config);
void liberar_todo(int n);
bool objetivo_global_cumplido();
bool elem_especies(t_list* especies, char* pokemon);
t_planificado* buscar_donador(t_entrenador* entrenador);
void destruir_vectores_de_semaforos();

t_planificado* elegir_proximo_a_ejecutar_SJFCD();

// Funciones SJF (pendientes a revision)
void enreadyar_al_mas_cercano_SJF(t_list* entrenadores, t_appeared_pokemon* appeared_pokemon);
void planificar_entrenadores_SJF();
void* ejecutar_entrenador_SJF(void* parametro);
void realizar_intercambios_SJF();
void intercambiar_pokemon_SJF(t_entrenador* entrenador, t_planificado* planificado);
void sacar_de_los_entrenadores_deadlock(t_entrenador* entrenador);
void modificar_estimacion_y_rafaga(t_entrenador* entrenador, u_int32_t rafaga);
t_planificado* elegir_proximo_a_ejecutar_SJF();
t_planificado* buscar_donador_SJF(t_entrenador* entrenador);
u_int32_t buscar_entrenador_con_donador_con_estimacion_mas_baja(t_list* donadores);
t_list* buscar_donadores_para_cada_entrenador();

#endif /* TEAM_H_ */
