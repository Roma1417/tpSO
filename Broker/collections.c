/*
 * entrenador.c
 *
 *  Created on: 1 may. 2020
 *      Author: utnso
 */

#include "collections.h"

#define min(a,b) (((a)<(b))?(a):(b))

t_memoria* crear_memoria(uint32_t tamanio){
	t_memoria* memoria = malloc(sizeof(t_memoria));

	memoria->base = malloc(tamanio);
	memoria->tamanio = tamanio;
	memoria->particiones = list_create();
	t_particion* particion_base = crear_particion(memoria->base, memoria->tamanio, false, crear_atributos_particion(0, 0, 0, 0, 0));
	list_add(memoria->particiones, particion_base);
	return memoria;
}

void destruir_memoria(t_memoria* memoria){
	free(memoria->base);


	list_clean_and_destroy_elements(memoria->particiones, destruir_particion);
	/*for(uint32_t i = 0; i < list_size(memoria->particiones); i++){
		t_particion* particion = list_get(memoria->particiones, i);
		destruir_particion(particion);
	}*/

	//list_destroy(memoria->particiones);
	free(memoria);
}


t_particion* crear_particion(void* base, uint32_t tamanio, bool ocupada, t_atributos_particion* atributos){
	t_particion* particion = malloc(sizeof(t_particion));

	particion->base = base;
	particion->tamanio = tamanio;
	particion->ocupada = ocupada;
	particion->atributos = atributos;

	return particion;
}

void destruir_particion(t_particion* particion){
	if (particion != NULL){
	destruir_atributos_particion(particion->atributos);
	}
	free(particion);
}


t_particion* buscar_particion(t_memoria* memoria, uint32_t id_mensaje){
	t_particion* particion_actual;

	for (uint32_t i = 0; i < list_size(memoria->particiones); i++){
		particion_actual = list_get(memoria->particiones, i);
		if (particion_actual->atributos->id_mensaje == id_mensaje) return particion_actual;
	}
	return NULL;
}



t_atributos_particion* crear_atributos_particion(uint32_t lru, uint32_t cola_mensajes, uint32_t id_mensaje, uint32_t id_correlativo, uint32_t stream_size){
	t_atributos_particion* atributos = malloc(sizeof(t_atributos_particion));
	atributos->lru = lru;
	atributos->cola_mensajes = cola_mensajes;
	atributos->id_mensaje = id_mensaje;
	atributos->id_correlativo = id_correlativo;
	atributos->stream_size = stream_size;
	atributos->suscriptores_enviados = list_create();
	atributos->suscriptores_confirmados = list_create();
	return atributos;
}

void destruir_atributos_particion(t_atributos_particion* atributos_particion){
	if (atributos_particion != NULL) {
	//list_destroy(atributos_particion->suscriptores_enviados);
	//list_destroy(atributos_particion->suscriptores_confirmados);



	}
	//free(atributos_particion);
}

t_suscriptor* crear_suscriptor(u_int32_t id, int32_t socket){

	t_suscriptor* suscriptor = malloc(sizeof(t_suscriptor));
	suscriptor -> id = id;
	suscriptor -> numero_socket = socket;

	return suscriptor;
}

t_suscriptor* generar_suscriptor(u_int32_t socket, uint32_t id_cola_mensajes){ //
	t_suscriptor* suscriptor = crear_suscriptor(generar_id_suscriptor(id_cola_mensajes), socket);

	return suscriptor;
}

t_list* obtener_lista_suscriptores(uint32_t id_cola_mensajes){
	return lista_suscriptores[id_cola_mensajes-1];
}



t_suscriptor* buscar_suscriptor(t_list* lista_suscriptores, u_int32_t id_suscriptor){

	for(u_int32_t x=0; x < list_size(lista_suscriptores); x++){
		t_suscriptor* aux_suscriptor = list_get(lista_suscriptores, x);

		if(aux_suscriptor->id == id_suscriptor) return aux_suscriptor;
	}


	return NULL;
}

u_int32_t generar_id_suscriptor(tipo_mensaje id_cola){

	u_int32_t id = generador_id_suscriptor[id_cola - 1]++;

	return id;
}

void destruir_suscriptor(void* suscriptor){
 //No hago nada, pero a lo mejor en algun momento sea de utilidad.
	free(suscriptor);
}


u_int32_t generar_id_mensaje(){

	u_int32_t id = generador_id_mensaje++;

	return id;
}

t_paquete* crear_paquete(u_int32_t id_mensaje, uint32_t id_correlativo, tipo_mensaje tipo_mensaje,t_buffer* buffer){
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->tipo_mensaje = tipo_mensaje;
	paquete->id_mensaje = id_mensaje;
	paquete->id_correlativo = id_correlativo;
	paquete->buffer = buffer;

	return paquete;
}

t_paquete* generar_paquete(t_particion* particion){
	uint32_t size = particion->atributos->stream_size;
	void* stream = malloc(size);
	memcpy(stream, particion->base, size);
	t_buffer* buffer = crear_buffer(size, stream);
	//printf("tipo mensaje: %d\n", particion->atributos->cola_mensajes);
	free(stream);
	return crear_paquete(particion->atributos->id_mensaje, particion->atributos->id_correlativo, particion->atributos->cola_mensajes, buffer);

}

void destruir_paquete(t_paquete* paquete){
	destruir_buffer(paquete->buffer);
	free(paquete);
}

t_buffer* crear_buffer(u_int32_t size, void* stream){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->stream = malloc(size);

	buffer->size = size;
	memcpy(buffer->stream, stream, size);;

	return buffer;
}

void destruir_buffer(t_buffer* buffer){
	free(buffer->stream);
	free(buffer);
}
