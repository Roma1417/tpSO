/*
 * entrenador.c
 *
 *  Created on: 1 may. 2020
 *      Author: utnso
 */

#include "memory.h"

int algoritmo_eleccion = 2;

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

void agregar_stream(t_memoria* memoria, void* stream, uint32_t tamanio){
	t_list* particiones = memoria->particiones;
	uint32_t indice_particion_seleccionada = buscar_indice_particion(particiones, max(tamanio, tamano_minimo_particion));
	agregar_particion(particiones, indice_particion_seleccionada, stream, max(tamanio, tamano_minimo_particion));
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
		uint32_t indice_particion_elegida;
		t_particion* particion_elegida = NULL;

				for (uint32_t i = 0; i < list_size(particiones); i++){
					t_particion* particion_actual = list_get(particiones, i);

					if(tamanio < particion_actual->tamanio && !(particion_actual->ocupada)){

						if(particion_elegida == NULL || particion_actual->tamanio < particion_elegida->tamanio){

							indice_particion_elegida = i;
							particion_elegida = list_get(particiones, indice_particion_elegida);
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


void agregar_particion(t_list* particiones, uint32_t indice, void* stream, uint32_t tamanio){
	t_particion* particion_seleccionada = list_get(particiones, indice);
	t_particion* particion_inicial = list_get(particiones, 0);

	t_particion* particion_libre = crear_particion(particion_seleccionada->base + tamanio, particion_seleccionada->tamanio - tamanio, false);
	list_add_in_index(particiones, indice + 1,particion_libre);

	memcpy(particion_seleccionada->base, stream, tamanio);
	particion_seleccionada->tamanio = tamanio;
	particion_seleccionada->ocupada =true;

	log_info(logger, "Se ha almacenado un mensaje dentro de la memoria. (Base: %d)", particion_seleccionada->base - particion_inicial->base);
}


void mostrar_memoria(t_memoria* memoria){
	for (uint32_t i = 0; i < list_size(memoria->particiones); i++){
		t_particion* particion_actual = list_get(memoria->particiones, i);
		printf("\nParticion %d:\n", i);
		printf("Base: %d\n", particion_actual->base);
		printf("Tamanio: %d\n", particion_actual->tamanio);
		printf("Está ocupada: %d\n", particion_actual->ocupada);
		printf("Contenido: %s\n",particion_actual->base);
	}
}

void* liberar_particion(t_memoria* memoria, uint32_t indice){
	t_list* particiones = memoria->particiones;
	t_particion* particion = list_get(particiones, indice);
	particion->ocupada = false;
	void* stream = malloc(particion->tamanio);
	memcpy(stream, particion->base, particion->tamanio);

	t_particion* particion_siguiente = list_get(particiones, indice + 1);
	if (particion_siguiente != NULL && particion_siguiente->ocupada == false){
		combinar_particiones(particiones, indice);
	}

	t_particion* particion_anterior = list_get(particiones, indice - 1);
	if (particion_anterior != NULL && particion_anterior->ocupada == false){
		combinar_particiones(particiones, indice - 1);
	}

	log_info(logger, "Se ha liberado un mensaje de la memoria. (Base: %p)", particion->base);
	return stream;
}

void combinar_particiones(t_list* particiones, uint32_t indice){
	t_particion* particion = list_get(particiones, indice);
	t_particion* particion_siguiente = list_get(particiones, indice + 1);

	particion->tamanio += particion_siguiente->tamanio;
	list_remove_and_destroy_element(particiones, indice + 1, free);
}

void compactar_memoria(t_memoria* memoria){
	t_list* particiones = memoria->particiones;

	for (uint32_t i = 0; i < list_size(particiones); i++){
		t_particion* particion_actual = list_get(particiones, i);
		if(particion_actual->ocupada == 1){
			uint32_t tamanio = particion_actual->tamanio;
			void* stream = liberar_particion(memoria, i);
			agregar_stream(memoria, stream, tamanio);
			free(stream);
		}
	}

	log_info(logger, "Se ha ejecutado una compactación de memoria.");

}

uint32_t max(uint32_t X, uint32_t Y){
	return X > Y ? X : Y;
}


