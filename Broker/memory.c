/*
 * entrenador.c
 *
 *  Created on: 1 may. 2020
 *      Author: utnso
 */

#include "memory.h"

#define max(a,b) (((a)>(b))?(a):(b))


t_memoria* crear_memoria(uint32_t tamanio){
	t_memoria* memoria = malloc(sizeof(t_memoria));
	memoria->base = malloc(tamanio);
	memoria->tamanio = tamanio;
	memoria->particiones = list_create();
	t_particion* particion_base = crear_particion(memoria->base, memoria->tamanio, false, 0, 0, 0, 0);
	list_add(memoria->particiones, particion_base);
	return memoria;
}

t_particion* crear_particion(void* base, uint32_t tamanio, bool ocupada, uint32_t tiempo_lru, uint32_t cola_mensajes, uint32_t id_mensaje, uint32_t id_correlativo){
	t_particion* particion = malloc(sizeof(t_particion));
	particion->base = base;
	particion->tamanio = tamanio;
	particion->ocupada = ocupada;
	particion->lru = tiempo_lru;
	particion->cola_mensajes = cola_mensajes;
	particion->id_mensaje = id_mensaje;
	particion->id_correlativo = id_correlativo;

	return particion;
}

void agregar_stream(t_memoria* memoria, void* stream, uint32_t tamanio, id_particion_libre algoritmo_particion_libre, uint32_t lru, uint32_t cola_mensajes, uint32_t id_mensaje, uint32_t id_correlativo, bool loguear){
	t_list* particiones = memoria->particiones;
	uint32_t indice_particion_seleccionada = buscar_indice_particion(particiones, max(tamanio, tamano_minimo_particion), algoritmo_particion_libre);

	while(indice_particion_seleccionada == -1){
		optimizar_memoria(memoria);
		indice_particion_seleccionada = buscar_indice_particion(particiones, max(tamanio, tamano_minimo_particion), algoritmo_particion_libre);
	}
	agregar_particion(particiones, indice_particion_seleccionada, stream, max(tamanio, tamano_minimo_particion), lru, cola_mensajes, id_mensaje, id_correlativo, loguear);
	if(tamanio < tamano_minimo_particion) limpiar_particion(particiones, indice_particion_seleccionada, tamanio);
}

uint32_t buscar_indice_particion(t_list* particiones, uint32_t tamanio, id_particion_libre algoritmo){

	switch(algoritmo){
	case FF:

		for (uint32_t i = 0; i < list_size(particiones); i++){
			t_particion* particion_actual = list_get(particiones, i);
			if(tamanio <= particion_actual->tamanio && !particion_actual->ocupada){
				return i;
			}
		}
		break;

	case BF:;
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





void agregar_particion(t_list* particiones, uint32_t indice, void* stream, uint32_t tamanio, uint32_t lru, uint32_t cola_mensajes, uint32_t id_mensaje, uint32_t id_correlativo, bool loguear){
	t_particion* particion_seleccionada = list_get(particiones, indice);
	t_particion* particion_inicial = list_get(particiones, 0);
	t_particion* particion_libre = crear_particion(particion_seleccionada->base + tamanio, particion_seleccionada->tamanio - tamanio, false, 0, 0, 0, 0);

	list_add_in_index(particiones, indice + 1,particion_libre);
	memcpy(particion_seleccionada->base, stream, tamanio);
	particion_seleccionada->tamanio = tamanio;
	particion_seleccionada->ocupada =true;
	particion_seleccionada->lru = lru;
	particion_seleccionada->cola_mensajes = cola_mensajes;
	particion_seleccionada->id_mensaje = id_mensaje;
	particion_seleccionada->id_correlativo = id_correlativo;;
	queue_push(cola_victimas, (void*) particion_seleccionada->id_mensaje);

	if (loguear) log_info(logger, "Se ha almacenado un mensaje dentro de la memoria. (Base: %d)", particion_seleccionada->base - particion_inicial->base);
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

void* liberar_particion(t_memoria* memoria, uint32_t indice, bool loguear){
	t_list* particiones = memoria->particiones;
	t_particion* particion = list_get(particiones, indice);
	void* base_particion_liberada = particion->base;

	particion->ocupada = false;
	void* stream = malloc(particion->tamanio);
	memcpy(stream, particion->base, particion->tamanio);

	t_particion* particion_siguiente = list_get(particiones, indice + 1);
	if (particion_siguiente != NULL && !particion_siguiente->ocupada){
		combinar_particiones(particiones, indice);
	}

	t_particion* particion_anterior = list_get(particiones, indice - 1);
	if (particion_anterior != NULL && !particion_anterior->ocupada){
		combinar_particiones(particiones, indice - 1);
	}

	if (loguear) log_info(logger, "Se ha liberado un mensaje de la memoria. (Base: %d)", base_particion_liberada - memoria->base);
	return stream;
}

void combinar_particiones(t_list* particiones, uint32_t indice){
	t_particion* particion = list_get(particiones, indice);
	t_particion* particion_siguiente = list_get(particiones, indice + 1);

	particion->tamanio += particion_siguiente->tamanio;
	list_remove_and_destroy_element(particiones, indice + 1, free);
}

void limpiar_particion(t_list* particiones, uint32_t indice, uint32_t tamanio){
	t_particion* particion = list_get(particiones, indice);
	char* sentinel = particion->base + tamanio;
	*sentinel = 0;
	//particion->base + tamanio = 0;
}

void compactar_memoria(t_memoria* memoria){
	t_list* particiones = memoria->particiones;

	for (uint32_t i = 0; i < list_size(particiones); i++){
		t_particion* particion_actual = list_get(particiones, i);
		if(particion_actual->ocupada == true){
			uint32_t tamanio = particion_actual->tamanio;
			uint32_t lru = particion_actual->lru;
			uint32_t cola_mensajes = particion_actual->cola_mensajes;
			uint32_t id_mensaje = particion_actual->id_mensaje;
			void* stream = liberar_particion(memoria, i, false);
			agregar_stream(memoria, stream, tamanio, FF, lru, cola_mensajes, id_mensaje, 0, false);
			free(stream);
		}
	}

	log_info(logger, "Se ha ejecutado una compactación de memoria.");

}

void optimizar_memoria(t_memoria* memoria){
	liberar_memoria(memoria);
}


void liberar_memoria(t_memoria* memoria){

	if (clock_compactacion--){
		uint32_t indice_victima = obtener_indice_particion_victima(memoria);
		liberar_particion(memoria, indice_victima, true);
	} else {
		compactar_memoria(memoria);
		clock_compactacion = frecuencia_compactacion;
	}

}

uint32_t obtener_indice_particion_victima(t_memoria* memoria){
	t_list* particiones = memoria -> particiones;
	t_particion* particion_victima;
	t_particion* particion_actual;


	switch (obtener_id_seleccion_victima(algoritmo_reemplazo)){
	case FIFO:;
		uint32_t id_particion_victima = (uint32_t) queue_pop(cola_victimas);
		for(uint32_t i=0; i<list_size(particiones); i++){
			particion_actual = list_get(particiones, i);
				if (id_particion_victima == particion_actual->id_mensaje){
					return i;
				}

			}
		break;
	case LRU:;
		uint32_t indice_victima = 0;
		particion_victima = NULL;
		for(uint32_t i=0; i<list_size(particiones); i++){
			particion_actual = list_get(particiones, i);
			if (particion_actual->ocupada && (particion_victima == NULL || particion_actual->lru < particion_victima->lru)){
				particion_victima = particion_actual;
				indice_victima = i;
			}
			}
		return indice_victima;
	}

	return -1;

}



/*uint32_t max(uint32_t X, uint32_t Y){
	return X > Y ? X : Y;
}*/

id_particion_libre obtener_id_particion_libre(char* nombre_algoritmo){
	if(string_equals_ignore_case(nombre_algoritmo,"FF")) return FF;
	if(string_equals_ignore_case(nombre_algoritmo,"BF")) return BF;
	return -1;
}

id_seleccion_victima obtener_id_seleccion_victima(char* nombre_algoritmo){
	if(string_equals_ignore_case(nombre_algoritmo,"FIFO")) return FIFO;
	if(string_equals_ignore_case(nombre_algoritmo,"LRU")) return LRU;
	return -1;
}

void generar_dump(t_memoria* memoria){
	t_list* particiones = memoria->particiones;
	t_particion* particion_actual = list_get(particiones, 0);
	void* base = particion_actual->base;

	FILE* dump = fopen("./broker.dump", "w");

	for(uint32_t i=0; i<list_size(particiones); i++){
		particion_actual = list_get(particiones, i);
		if(particion_actual->ocupada){
	fprintf(dump, "Particion %d: %p - %p.\t[X]\tSize: %db\tLRU:%d\tCola %d\tID:%d\n", i, particion_actual->base - base, particion_actual->base + particion_actual->tamanio - base, particion_actual->tamanio, particion_actual->lru, particion_actual->cola_mensajes, particion_actual->id_mensaje);
		} else {
			fprintf(dump, "Particion %d: %p - %p.\t[L]\tSize: %db\n", i, particion_actual->base - base, particion_actual->base + particion_actual->tamanio - base, particion_actual->tamanio);
		}
		}

	fclose(dump);
}
