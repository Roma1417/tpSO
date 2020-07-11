/*
 * entrenador.c
 *
 *  Created on: 1 may. 2020
 *      Author: utnso
 */

#include "memory.h"

#define max(a,b) (((a)>(b))?(a):(b))



t_particion* agregar_stream(t_memoria* memoria, void* stream, uint32_t tamanio, id_particion_libre algoritmo_particion_libre, t_atributos_particion* atributos, bool loguear){
	t_list* particiones = memoria->particiones;
	uint32_t indice_particion_seleccionada = obtener_indice_particion_libre(particiones, max(tamanio, tamano_minimo_particion), algoritmo_particion_libre);

	while(indice_particion_seleccionada == -1){
		optimizar_memoria(memoria);
		indice_particion_seleccionada = obtener_indice_particion_libre(particiones, max(tamanio, tamano_minimo_particion), algoritmo_particion_libre);
	}
	t_particion* particion_agregada = agregar_particion(particiones, indice_particion_seleccionada, stream, max(tamanio, tamano_minimo_particion), atributos, loguear);
	if(tamanio < particion_agregada->tamanio) limpiar_particion(particiones, indice_particion_seleccionada, tamanio);

	return particion_agregada;
}


uint32_t obtener_indice_particion_libre(t_list* particiones, uint32_t tamanio, id_particion_libre algoritmo){

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


t_particion* agregar_particion(t_list* particiones, uint32_t indice, void* stream, uint32_t tamanio, t_atributos_particion* atributos, bool loguear){
	t_particion* particion_seleccionada;
	t_particion* particion_inicial = list_get(particiones, 0);

	switch(obtener_id_algoritmo_memoria(algoritmo_memoria)){
	case PD:
		particion_seleccionada = list_get(particiones, indice);
		if (particion_seleccionada->tamanio - tamanio != 0){
			t_particion* particion_libre = crear_particion(particion_seleccionada->base + tamanio, particion_seleccionada->tamanio - tamanio, false, crear_atributos_particion(0, 0, 0, 0, 0));
			list_add_in_index(particiones, indice + 1,particion_libre);
		}
		particion_seleccionada->tamanio = tamanio;
		break;
	case BS:
		particion_seleccionada = obtener_particion_ajustada(particiones, indice, tamanio);
		break;
	}


	memcpy(particion_seleccionada->base, stream, tamanio);


	particion_seleccionada->ocupada =true;
	//destruir_atributos_particion(particion_seleccionada->atributos);
	particion_seleccionada->atributos = atributos;


	if (loguear) {
		queue_push(cola_victimas, (void*) particion_seleccionada->atributos->id_mensaje);
		log_info(logger, "Se ha almacenado un mensaje dentro de la memoria. (Base: %d)", particion_seleccionada->base - particion_inicial->base);
	}

	return particion_seleccionada;
}


void mostrar_memoria(t_memoria* memoria){
	for (uint32_t i = 0; i < list_size(memoria->particiones); i++){
		t_particion* particion_actual = list_get(memoria->particiones, i);
		printf("\nParticion %d:\n", i);
		printf("Base: %d\n", (int) particion_actual->base);
		printf("Tamanio: %d\n", particion_actual->tamanio);
		printf("Está ocupada: %d\n", particion_actual->ocupada);
		printf("Contenido: %s\n", (char*) particion_actual->base);
	}
}


void* liberar_particion(t_memoria* memoria, uint32_t indice, bool loguear){
	t_list* particiones = memoria->particiones;
	t_particion* particion = list_get(particiones, indice);
	void* base_particion_liberada = particion->base;

	particion->ocupada = false;
	void* stream = malloc(particion->tamanio);
	memcpy(stream, particion->base, particion->tamanio);



	switch(obtener_id_algoritmo_memoria(algoritmo_memoria)){
	case PD:;
		t_particion* particion_siguiente = list_get(particiones, indice + 1);
		if (particion_siguiente != NULL && !particion_siguiente->ocupada){
			combinar_particiones(particiones, indice);
		}
		t_particion* particion_anterior = list_get(particiones, indice - 1);
		if (particion_anterior != NULL && !particion_anterior->ocupada){
			combinar_particiones(particiones, indice - 1);

		}
		break;
	case BS:
		consolidar_particiones_bs(particiones, indice);
		break;

	}



	if (loguear) log_info(logger, "Se ha liberado un mensaje de la memoria. (Base: %d)", base_particion_liberada - memoria->base);
	return stream;
}


void combinar_particiones(t_list* particiones, uint32_t indice){
	t_particion* particion = list_get(particiones, indice);
	t_particion* particion_siguiente = list_get(particiones, indice + 1);

	particion->tamanio += particion_siguiente->tamanio;
	list_remove(particiones, indice + 1);
	destruir_particion(particion_siguiente);
}


void limpiar_particion(t_list* particiones, uint32_t indice, uint32_t tamanio){
	t_particion* particion = list_get(particiones, indice);
	char* sentinel = particion->base + tamanio;
	*sentinel = 0;
	//particion->base + tamanio = 0;
}


void compactar_memoria(t_memoria* memoria){
	t_list* particiones = memoria->particiones;
	printf("Compactando \n");
	for (uint32_t i = 0; i < list_size(particiones); i++){
		t_particion* particion_actual = list_get(particiones, i);
		if(particion_actual->ocupada == true){
			uint32_t tamanio = particion_actual->tamanio;
			t_atributos_particion* atributos = particion_actual->atributos;
			void* stream = liberar_particion(memoria, i, false);
			agregar_stream(memoria, stream, tamanio, FF, atributos, false);
			free(stream);
		}
	}

	log_info(logger, "Se ha ejecutado una compactación de memoria.");

}


void optimizar_memoria(t_memoria* memoria){
	if (clock_compactacion-- || obtener_id_algoritmo_memoria(algoritmo_memoria) == BS){
		liberar_memoria(memoria);
	} else {
		compactar_memoria(memoria);
		clock_compactacion = frecuencia_compactacion;
	}
}


void liberar_memoria(t_memoria* memoria){
	uint32_t indice_victima = obtener_indice_particion_victima(memoria);
	void* stream_a_liberar = liberar_particion(memoria, indice_victima, true);
	free(stream_a_liberar);
}


t_particion* dividir_particion(uint32_t indice_particion, t_list* particiones){
	t_particion* particion_a_dividir = list_get(particiones, indice_particion);
	particion_a_dividir->tamanio /= 2;

	t_particion* nueva_particion = crear_particion(particion_a_dividir->base + particion_a_dividir->tamanio, particion_a_dividir->tamanio, false, particion_a_dividir->atributos);
	list_add_in_index(particiones, indice_particion + 1, nueva_particion);

	return particion_a_dividir;
}


uint32_t potencia_de_2_mas_cercana(uint32_t numero){
	uint32_t potencia_mas_cercana = 1;

	while(numero > potencia_mas_cercana){
		potencia_mas_cercana *= 2;
	}

	return potencia_mas_cercana;
}


t_particion* obtener_particion_ajustada(t_list* particiones, uint32_t indice, uint32_t tamanio){
	t_particion* particion_seleccionada = list_get(particiones, indice);
	while(particion_seleccionada->tamanio != potencia_de_2_mas_cercana(tamanio)){
		printf("%d\t%d\n", potencia_de_2_mas_cercana(tamanio), particion_seleccionada->tamanio);
		//sleep(2);
		particion_seleccionada = dividir_particion(indice, particiones);
	}
	return particion_seleccionada;
}


void consolidar_particiones_bs(t_list* particiones, uint32_t indice){
	t_particion* particion = list_get(particiones, indice);
	t_particion* particion_siguiente = list_get(particiones, indice + 1);
	t_particion* particion_anterior = list_get(particiones, indice - 1);
	t_particion* particion_inicial = list_get(particiones, 0);


	if (particion_siguiente != NULL && !particion_siguiente->ocupada && particion->tamanio == particion_siguiente->tamanio && ((particion->base - particion_inicial->base) ^ (particion->tamanio)) == (particion_siguiente->base - particion_inicial->base)){
		combinar_particiones(particiones, indice);
		consolidar_particiones_bs(particiones, indice);
	}
	if (particion_anterior != NULL && !particion_anterior->ocupada && particion->tamanio == particion_anterior->tamanio && ((particion->base - particion_inicial->base) ^ (particion->tamanio)) == (particion_anterior->base - particion_inicial->base)){
		combinar_particiones(particiones, indice - 1);
		consolidar_particiones_bs(particiones, indice - 1);
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
		if (id_particion_victima == particion_actual->atributos->id_mensaje){
			return i;
		}

	}
	break;
	case LRU:;
	uint32_t indice_victima = 0;
	particion_victima = NULL;
	for(uint32_t i=0; i<list_size(particiones); i++){
		particion_actual = list_get(particiones, i);
		if (particion_actual->ocupada && (particion_victima == NULL || particion_actual->atributos->lru < particion_victima->atributos->lru)){
			particion_victima = particion_actual;
			indice_victima = i;
		}
	}
	return indice_victima;
	}

	return -1;

}


id_algoritmo_memoria obtener_id_algoritmo_memoria(char* nombre_algoritmo){
	if(string_equals_ignore_case(nombre_algoritmo,"PARTICIONES")) return PD;
	if(string_equals_ignore_case(nombre_algoritmo,"BS")) return BS;
	return -1;
}


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
			fprintf(dump, "Particion %d: %p - %p.\t[X]\tSize: %db\tLRU:%d\tCola %d\tID:%d\n", i, (void*) (particion_actual->base - base), (void*) (particion_actual->base + particion_actual->tamanio - base), particion_actual->tamanio, particion_actual->atributos->lru, particion_actual->atributos->cola_mensajes, particion_actual->atributos->id_mensaje);
		} else {
			fprintf(dump, "Particion %d: %p - %p.\t[L]\tSize: %db\n", i, (void*) (particion_actual->base - base), (void*) (particion_actual->base + particion_actual->tamanio - base), particion_actual->tamanio);
		}
	}

	fclose(dump);
}
