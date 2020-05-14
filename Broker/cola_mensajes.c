/*
 * entrenador.c
 *
 *  Created on: 1 may. 2020
 *      Author: utnso
 */

#include "cola_mensajes.h"




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
	free(self);

}

void set_cola_mensajes(char* nombre_cola){

	t_config* config = config_create("colas_mensajes.config");
	t_cola_mensajes* cola = cola_mensajes_create(obtener_tipo_mensaje(nombre_cola));
	char direccion[12];

	sprintf(direccion, "%d", (int)cola);
	config_set_value(config, nombre_cola, direccion);
	config_save(config);
	config_destroy(config);
}

void* get_cola_mensajes(char* nombre_cola){

	t_config* config = config_create("colas_mensajes.config");

	void* direccion_cola = (void*)config_get_int_value(config, nombre_cola);
	config_destroy(config);
	return (void*) direccion_cola;

}


t_suscriptor* crear_suscriptor(char* ip, char* puerto){

	t_suscriptor* suscriptor = malloc(sizeof(t_suscriptor));
	suscriptor -> ip = ip;
	suscriptor -> puerto = puerto;

	return suscriptor;
}


void agregar_suscriptor(int32_t* suscriptor, t_cola_mensajes* cola_mensajes){

	list_add(cola_mensajes->suscriptores, suscriptor);
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

