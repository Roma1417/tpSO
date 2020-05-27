/*
 * entrenador.c
 *
 *  Created on: 1 may. 2020
 *      Author: utnso
 */

#include "cola_mensajes.h"

u_int32_t generador_id_suscriptores[6] = {1,1,1,1,1,1};


t_cola_mensajes* cola_mensajes_create(tipo_mensaje id){

	t_cola_mensajes* cola_mensajes = malloc(sizeof(t_cola_mensajes));
	t_queue* mensajes = queue_create();
	t_list* suscriptores = list_create();

	cola_mensajes -> id = id;
	cola_mensajes -> mensajes = mensajes;
	cola_mensajes -> suscriptores = suscriptores;

		return cola_mensajes;
}

void cola_mensajes_destroy(t_cola_mensajes* self){

	queue_destroy(self -> mensajes);
	list_destroy(self -> suscriptores);
	printf("Ward\n");
	free(self);

}


void* get_cola_mensajes(tipo_mensaje tipo){

	return colas_mensajes[tipo - 1];

}


t_suscriptor* crear_suscriptor(u_int32_t id, u_int32_t socket){

	t_suscriptor* suscriptor = malloc(sizeof(t_suscriptor));
	suscriptor -> id = id;
	suscriptor -> socket = socket;

	return suscriptor;
}


t_suscriptor* agregar_suscriptor(u_int32_t socket, t_cola_mensajes* cola_mensajes){ //Agrega suscriptor a una lista y retorna la id, que seria la posicion en la lista +1

	t_suscriptor* suscriptor = crear_suscriptor(generar_id_suscriptor(cola_mensajes->id), socket);

	list_add(cola_mensajes->suscriptores, suscriptor);

	return suscriptor;
}


t_suscriptor* actualizar_suscriptor(u_int32_t socket, t_cola_mensajes* cola_mensajes, u_int32_t id_suscriptor){

	t_suscriptor* suscriptor = buscar_suscriptor(cola_mensajes, id_suscriptor);

	suscriptor->socket = socket;

	return suscriptor;

}

t_suscriptor* buscar_suscriptor(t_cola_mensajes* cola_mensajes, u_int32_t id_suscriptor){

	t_list* lista_suscriptores = cola_mensajes->suscriptores;
	for(u_int32_t x=0; x < list_size(lista_suscriptores); x++){
		t_suscriptor* aux_suscriptor = list_get(lista_suscriptores, x);

		if(aux_suscriptor->id == id_suscriptor) return aux_suscriptor;
	}

	printf("Error buscando al suscriptor\n");

	return NULL;
}

u_int32_t generar_id_suscriptor(tipo_mensaje id_cola){
	return generador_id_suscriptores[id_cola - 1]++;
}

void eliminar_suscriptor(int32_t* suscriptor, t_cola_mensajes* cola_mensajes){

	t_list* suscriptores = cola_mensajes->suscriptores;

	for (u_int32_t i=0; i<list_size(suscriptores); i++){
		if (list_get(suscriptores, i) == suscriptor){
			list_remove(cola_mensajes->suscriptores,i);
		}
	}
}


t_mensaje* crear_mensaje(u_int32_t size, void* stream){
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->stream = malloc(size);

	mensaje->size = size;
	mensaje->stream = stream;

	return mensaje;
}


void agregar_mensaje(t_mensaje* mensaje, t_cola_mensajes* cola_mensajes){

	queue_push(cola_mensajes->mensajes, mensaje);
}

