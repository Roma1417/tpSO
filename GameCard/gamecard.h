/*
 * gamecard.h
 *
 *  Created on: 28 may. 2020
 *      Author: utnso
 */

#ifndef GAMECARD_H_
#define GAMECARD_H_

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "utils.h"

typedef struct {
	uint32_t tiempo_de_reintento_conexion;
	uint32_t tiempo_de_reintento_operacion;
	char* punto_montaje_tallgrass;
	char* ip_broker;
	char* puerto_broker;

}t_config_gamecard;

pthread_t hilo_new;
pthread_t hilo_get;
pthread_t hilo_catch;
t_config_gamecard* config_gamecard;
FILE * archivo_metadata;
FILE * archivo_bitmap;

t_config* leer_config();
t_log* iniciar_logger();
void suscribirse_a_colas();
void* suscribirse(void* cola);
t_config_gamecard* construir_config_gamecard(t_config* config);
char* id_cola_mensajes(char* msg);
void crear_archivos();
void crear_directorios();


#endif /* GAMECARD_H_ */
