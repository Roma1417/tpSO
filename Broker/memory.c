/*
 * entrenador.c
 *
 *  Created on: 1 may. 2020
 *      Author: utnso
 */

#include "memory.h"

int algoritmo_eleccion = 1;

t_memoria* crear_memoria(uint32_t tamanio){
	t_memoria* memoria = malloc(sizeof(t_memoria));
	memoria->base = malloc(tamanio);
	memoria->tamanio = tamanio;
	memoria->particiones = list_create();
	t_particion* particion_base = crear_particion(memoria->base, memoria->tamanio, false);
	list_add(memoria->particiones, particion_base);
	return memoria;
}

t_particion* crear_particion(void* base, uint32_t tamanio, bool ocupada){
	t_particion* particion = malloc(sizeof(t_particion));
	particion->base = base;
	particion->tamanio = tamanio;
	particion->ocupada = ocupada;

	return particion;
}

void agregar_stream(t_memoria* memoria, void* stream){
	t_list* particiones = memoria->particiones;
	uint32_t indice_particion_seleccionada = buscar_indice_particion(particiones, strlen(stream));
	agregar_particion(particiones, indice_particion_seleccionada, stream);
}

uint32_t buscar_indice_particion(t_list* particiones, uint32_t tamanio){

	switch(algoritmo_eleccion){
	case 1:

		for (uint32_t i = 0; i < list_size(particiones); i++){
			t_particion* particion_actual = list_get(particiones, i);
			if(tamanio < particion_actual->tamanio && !particion_actual->ocupada){
				return i;
			}
		}
		break;

	case 2:;
		uint32_t indice_particion_elegida = NULL;

				for (uint32_t i = 0; i < list_size(particiones); i++){
					t_particion* particion_actual = list_get(particiones, i);
					t_particion* particion_elegida = list_get(particiones, indice_particion_elegida);

					if(tamanio < particion_actual->tamanio && !particion_actual->ocupada){

						if(particion_actual->tamanio < particion_elegida->tamanio || particion_elegida == NULL){
							indice_particion_elegida = i;
						}
					}
				}
				return indice_particion_elegida;
				break;
	default:
		break;
	}
	return -1;

}


void agregar_particion(t_list* particiones, uint32_t indice, void* stream){
	t_particion* particion_seleccionada = list_get(particiones, indice);
	uint32_t tamanio= strlen(stream);

	t_particion* particion_libre = crear_particion(particion_seleccionada->base + tamanio, particion_seleccionada->tamanio - tamanio, false);
	list_add_in_index(particiones, indice + 1,particion_libre);

	memcpy(particion_seleccionada->base, stream, tamanio);
	particion_seleccionada->tamanio = tamanio;
	particion_seleccionada->ocupada =true;
}


void mostrar_memoria(t_memoria* memoria){
	for (uint32_t i = 0; i < list_size(memoria->particiones); i++){
		t_particion* particion_actual = list_get(memoria->particiones, i);
		printf("Particion %d\n", i);
		printf("Base: %d\n", particion_actual->base);
		printf("Tamanio: %d\n", particion_actual->tamanio);
		printf("Está ocupada: %d\n", particion_actual->ocupada);
		printf("Contenido: %s\n",particion_actual->base);
	}
}
