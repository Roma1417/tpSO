/*
 * gamecard.h
 *
 *  Created on: 28 may. 2020
 *      Author: utnso
 */

#ifndef GAMECARD_H_
#define GAMECARD_H_

#include <commons/collections/list.h>
#include <commons/string.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "utils.h"



pthread_t hilo_new;
pthread_t hilo_get;
pthread_t hilo_catch;
pthread_t hilo_servidor;
t_config* config;



t_config* leer_config();
t_log* iniciar_logger();
void suscribirse_a_colas();
void* suscribirse(void* cola);
t_config_gamecard* construir_config_gamecard(t_config* config);
char* id_cola_mensajes(char* msg);
void crear_archivos();
void crear_directorios();


#endif /* GAMECARD_H_ */
