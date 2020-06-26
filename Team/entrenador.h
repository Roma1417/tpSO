/*
 * entrenador.h
 *
 *  Created on: 1 may. 2020
 *      Author: utnso
 */

#ifndef ENTRENADOR_H_
#define ENTRENADOR_H_

#include <stdlib.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "posicion.h"
#include "appeared_pokemon.h"

typedef enum{
	NEW = 1,
	READY = 2,
	EXEC = 3,
	BLOCK = 4,
	EXIT = 5,
} t_estado;

typedef struct {

	t_posicion* posicion;
	t_list* pokemon_obtenidos;
	t_list* objetivos;
	t_list* objetivos_faltantes;
	t_list* pokemon_inservibles;
	t_estado estado;
	pthread_t hilo;
	u_int32_t capturas_disponibles;
	u_int32_t indice;
	u_int32_t id_caught;
	u_int32_t resultado_caught;
	bool puede_pasar_a_ready;
	u_int32_t rafaga;
	u_int32_t ciclos_cpu;

	float estimacion;
	u_int32_t rafaga_anterior;

} t_entrenador;



void cambiar_estado(t_entrenador* entrenador, t_estado estado);
bool puede_ser_planificado(void* parametro);
t_entrenador* entrenador_create(t_posicion* posicion, t_list* pokemon_obtenidos,t_list* objetivos, u_int32_t indice, float estimacion_inicial);
t_list* get_objetivos(t_entrenador* entrenador);
t_list* get_objetivos_faltantes(t_entrenador* entrenador);
void entrenador_destroy(t_entrenador* entrenador);
bool puede_seguir_atrapando(t_entrenador* entrenador);
void decrementar_capturas_disponibles(t_entrenador* entrenador);
void set_hilo(t_entrenador* entrenador, pthread_t hilo);
void cambiar_condicion_ready(t_entrenador* entrenador);
void atrapar(t_entrenador* entrenador, t_appeared_pokemon* appeared_pokemon);
bool cumplio_su_objetivo(void* parametro);
bool no_cumplio_su_objetivo(void* parametro);
void intercambiar(t_entrenador* entrenador, char* objetivo, char* inservible);
void set_estimacion(t_entrenador* entrenador, float estimacion);
void set_rafaga_anterior(t_entrenador* entrenador, u_int32_t rafaga_anterior);


#endif /* ENTRENADOR_H_ */
