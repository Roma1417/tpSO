/*
 * entrenador.h
 *
 *  Created on: 1 may. 2020
 *      Author: utnso
 */

#ifndef MEMORY_H_
#define MEMORY_H_

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/string.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include<time.h>
#include<string.h>
#include<pthread.h>
#include"collections.h"


typedef enum{
	PD = 1,
	BS = 2,
}id_algoritmo_memoria;

typedef enum{
	FF = 1,
	BF = 2,
}id_particion_libre;

typedef enum{
	FIFO = 1,
	LRU = 2,
}id_seleccion_victima;




uint32_t tamano_minimo_particion;
char* algoritmo_memoria;
char* algoritmo_particion_libre;
char* algoritmo_reemplazo;
uint32_t frecuencia_compactacion;
char* log_file;
t_config* config;
t_log* logger;
t_queue* cola_victimas;
uint32_t timer_lru;
uint32_t clock_compactacion;



t_particion* agregar_stream(t_memoria*, void*, uint32_t, id_particion_libre, t_atributos_particion*, bool);
t_particion* agregar_particion(t_list*, uint32_t, void*, uint32_t, t_atributos_particion*, bool);
uint32_t obtener_indice_particion_libre(t_list*, uint32_t, uint32_t);

void mostrar_memoria(t_memoria*);
void destruir_memoria(t_memoria*);

void* liberar_particion(t_memoria*, uint32_t, bool);
void combinar_particiones(t_list*, uint32_t);
void limpiar_particion(t_list*, uint32_t, uint32_t);

void optimizar_memoria(t_memoria*);
void compactar_memoria(t_memoria*);
void liberar_memoria(t_memoria*);

t_particion* dividir_particion(uint32_t, t_list*);
uint32_t potencia_de_2_mas_cercana(uint32_t);
t_particion* obtener_particion_ajustada(t_list*, uint32_t, uint32_t);
void consolidar_particiones_bs(t_list* particiones, uint32_t indice);

uint32_t obtener_indice_particion_victima(t_memoria*);
id_algoritmo_memoria obtener_id_algoritmo_memoria(char*);
id_particion_libre obtener_id_particion_libre(char*);
id_seleccion_victima obtener_id_seleccion_victima(char*);

void generar_dump(t_memoria*);

#endif /* MEMORY_H_ */
