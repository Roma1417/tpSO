/*
 * entrenador.c
 *
 *  Created on: 1 may. 2020
 *      Author: utnso
 */

#include "collections.h"


t_cola_mensajes* crear_cola_mensajes(tipo_mensaje id){

	t_cola_mensajes* cola_mensajes = malloc(sizeof(t_cola_mensajes));
	t_list* mensajes = list_create();
	t_list* suscriptores = list_create();

	cola_mensajes -> id = id;
	cola_mensajes -> mensajes = mensajes;
	cola_mensajes -> suscriptores = suscriptores;

	return cola_mensajes;
}

void destruir_cola_mensajes(t_cola_mensajes* self){

	list_destroy(self -> mensajes);
	list_destroy(self -> suscriptores);
	printf("Ward\n");
	free(self);

}

void* get_cola_mensajes(tipo_mensaje tipo){

	return colas_mensajes[tipo - 1];

}

t_suscriptor* crear_suscriptor(u_int32_t id, int32_t socket){

	t_suscriptor* suscriptor = malloc(sizeof(t_suscriptor));
	suscriptor -> id = id;
	suscriptor -> numero_socket = socket;

	return suscriptor;
}

t_suscriptor* generar_suscriptor(u_int32_t socket, t_cola_mensajes* cola_mensajes){ //Agrega suscriptor a una lista y retorna la id, que seria la posicion en la lista +1
	t_suscriptor* suscriptor = crear_suscriptor(generar_id_suscriptor(cola_mensajes->id), socket);



	return suscriptor;
}




t_suscriptor* buscar_suscriptor(t_list* lista_suscriptores, u_int32_t id_suscriptor){

	for(u_int32_t x=0; x < list_size(lista_suscriptores); x++){
		t_suscriptor* aux_suscriptor = list_get(lista_suscriptores, x);

		if(aux_suscriptor->id == id_suscriptor) return aux_suscriptor;
	}

	printf("Error buscando al suscriptor\n");

	return NULL;
}

u_int32_t generar_id_suscriptor(tipo_mensaje id_cola){
	//wait(mutex_id_suscriptor);
	u_int32_t id = generador_id_suscriptor[id_cola - 1]++;
	//signal(mutex_id_suscriptor);
	return id;
}

t_mensaje* crear_mensaje(t_paquete* paquete){
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));

	mensaje->paquete = paquete;
	mensaje->suscriptores_enviados = list_create();
	mensaje->suscriptores_confirmados = list_create();

	return mensaje;
}

t_mensaje* generar_mensaje(int32_t id, tipo_mensaje tipo_mensaje, int32_t size, void* stream){
	t_buffer* buffer = crear_buffer(size, stream);
	t_paquete* paquete = crear_paquete(id, tipo_mensaje, buffer);
	t_mensaje* mensaje = crear_mensaje(paquete);
	return mensaje;
}

void destruir_mensaje(t_mensaje* mensaje){
	destruir_paquete(mensaje->paquete);
	list_destroy(mensaje->suscriptores_enviados);
	list_destroy(mensaje->suscriptores_confirmados);
	free(mensaje);
}

t_mensaje* buscar_mensaje(t_list* lista_mensajes, u_int32_t id_mensaje){
	printf("ward 2.1 \n");
	for(u_int32_t x=0; x < list_size(lista_mensajes); x++){
		t_mensaje* aux_mensaje = list_get(lista_mensajes, x);
		printf("ward 2.2 \n");
		if(aux_mensaje->paquete->id_mensaje == id_mensaje) return aux_mensaje;
	}

	printf("Error buscando al mensaje\n");

	return NULL;
}

u_int32_t generar_id_mensaje(){
	//wait(mutex_id_mensaje);
	u_int32_t id = generador_id_mensaje++;
	//signal(mutex_id_mensaje);
	return id;
}

t_paquete* crear_paquete(u_int32_t id_mensaje, tipo_mensaje tipo_mensaje,t_buffer* buffer){
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->tipo_mensaje = tipo_mensaje;
	paquete->id_mensaje = id_mensaje;
	paquete->buffer = buffer;

	return paquete;
}

void destruir_paquete(t_paquete* paquete){
	destruir_buffer(paquete->buffer);
	free(paquete);
}

t_buffer* crear_buffer(u_int32_t size, void* stream){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->stream = malloc(size);

	buffer->size = size;
	buffer->stream = stream;

	return buffer;
}

void destruir_buffer(t_buffer* buffer){
	free(buffer->stream);
	free(buffer);
}
