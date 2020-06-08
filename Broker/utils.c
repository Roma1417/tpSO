/*
 * conexiones.c
 *
 *  Created on: 3 mar. 2019
 *      Author: utnso
 */

#include "utils.h"



void iniciar_servidor(void)
{
	int socket_servidor;

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	char* ip = consultar_config_por_string("./broker.config", "IP_BROKER");
	char* puerto = consultar_config_por_string("./broker.config", "PUERTO_BROKER");

	getaddrinfo(ip, puerto, &hints, &servinfo);

	for (p=servinfo; p != NULL; p = p->ai_next)
	{
		if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
			continue;

		if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
			close(socket_servidor);
			continue;
		}
		break;
	}

	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);

	while(1)
		esperar_cliente(socket_servidor);
}


void esperar_cliente(int socket_servidor)
{
	struct sockaddr_in dir_cliente;

	u_int32_t tam_direccion = sizeof(struct sockaddr_in); //capaz no sea unsigned

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
	//agregar log de conexion
	pthread_create(&thread,NULL,(void*)serve_client,&socket_cliente);
	pthread_join(thread, NULL);

}


void serve_client(int* socket)
{
	int cod_op;
	if(recv(*socket, &cod_op, sizeof(int), MSG_WAITALL) == -1)
		cod_op = -1;
	process_request(cod_op, *socket);
}


void process_request(int cod_op, int cliente_fd) {
	t_cola_mensajes* cola_mensajes;

	//u_int32_t id_mensaje = recibir_entero(cliente_fd);


	switch (cod_op) {
	case NEW_POKEMON:
	case APPEARED_POKEMON:
	case CATCH_POKEMON:
	case CAUGHT_POKEMON:
	case GET_POKEMON:
	case LOCALIZED_POKEMON:
		printf("Recibí un mensaje de tipo %s\n", obtener_tipo_mensaje_string(cod_op));

		int32_t size;
		void* stream = (void*)recibir_cadena(cliente_fd, &size);
		printf("Parametro cadena %s\n",stream+4);

		t_buffer* buffer = crear_buffer(size, stream);
		t_paquete* paquete = crear_paquete(generar_id_mensaje(), cod_op, buffer);
		t_mensaje* mensaje = crear_mensaje(paquete);
		cola_mensajes = get_cola_mensajes(cod_op);

		list_add(cola_mensajes->mensajes, mensaje);

		printf("Asignado el mensaje de ID %d a la cola %s\n", mensaje->paquete->id_mensaje, obtener_tipo_mensaje_string(cola_mensajes->id));

		printf("El tamaño de la cola de mensajes ahora es %d\n", list_size(cola_mensajes->mensajes));

		enviar_a_suscriptores(mensaje, cola_mensajes->suscriptores);

		printf("GG\n");

		break;

	case SUSCRIPTOR:

		printf("Recibí un mensaje de tipo SUSCRIPTOR\n");

		size = recibir_entero(cliente_fd);
		printf("La size recibida fue: %d\n", size);

		int32_t cola_size;
		char* cola_nombre = recibir_cadena(cliente_fd, &cola_size);
		printf("La cola recibida fue: %s\n", cola_nombre);

		u_int32_t id_suscriptor = recibir_entero(cliente_fd);
		printf("La id recibida fue: %d\n", id_suscriptor);

		cola_mensajes = get_cola_mensajes(obtener_tipo_mensaje(cola_nombre));


		if(id_suscriptor == 0){
			agregar_suscriptor(cliente_fd, cola_mensajes);
		} else {
			actualizar_suscriptor(cliente_fd, cola_mensajes, id_suscriptor);
		}

		printf("Cantidad de subs en la cola %s: %d\n", cola_nombre, list_size(cola_mensajes->suscriptores));



		break;

	case CONFIRMAR:

		printf("Recibí un mensaje de tipo CONFIRMAR\n");
		size = recibir_entero(cliente_fd);
		printf("La size recibida fue: %d\n", size);
		u_int32_t id_cola_mensajes = recibir_entero(cliente_fd);
		printf("La cola recibida fue: %d\n", id_cola_mensajes);
		u_int32_t id_mensaje = recibir_entero(cliente_fd);
		printf("La mensaje recibida fue: %d\n", id_mensaje);
		u_int32_t id_suscriptor_confirmado = recibir_entero(cliente_fd);
		printf("La suscriptor recibida fue: %d\n", id_suscriptor_confirmado);
		printf("ward 1 \n");
		cola_mensajes = get_cola_mensajes(id_cola_mensajes);
		printf("ward 2 \n");
		t_mensaje* mensaje_confirmado = buscar_mensaje(cola_mensajes->mensajes, id_mensaje);
		printf("ward 3 \n");
		t_suscriptor* suscriptor_confirmado = buscar_suscriptor(mensaje_confirmado->suscriptores_enviados, id_suscriptor_confirmado);

		list_add(mensaje_confirmado->suscriptores_confirmados, suscriptor_confirmado);

		break;
	case 0:
		pthread_exit(NULL);
	case -1:
		pthread_exit(NULL);
	}
}


int recibir_entero(int socket_cliente){

	int entero;

	recv(socket_cliente, &entero, sizeof(int), MSG_WAITALL);

	return entero;
}


void* recibir_cadena(int socket_cliente, int* size)
{
	void * cadena;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	cadena = malloc(*size);
	recv(socket_cliente, cadena, *size, MSG_WAITALL);

	return cadena;
}


tipo_mensaje obtener_tipo_mensaje(char* tipo){
	if(string_equals_ignore_case(tipo,"NEW_POKEMON")) return NEW_POKEMON;
	if(string_equals_ignore_case(tipo,"APPEARED_POKEMON")) return APPEARED_POKEMON;
	if(string_equals_ignore_case(tipo,"CATCH_POKEMON")) return CATCH_POKEMON;
	if(string_equals_ignore_case(tipo,"CAUGHT_POKEMON")) return CAUGHT_POKEMON;
	if(string_equals_ignore_case(tipo,"GET_POKEMON")) return GET_POKEMON;
	if(string_equals_ignore_case(tipo, "LOCALIZED_POKEMON")) return LOCALIZED_POKEMON;
	if(string_equals_ignore_case(tipo,"CONFIRMAR")) return CONFIRMAR;
	return -1;
}


char* obtener_tipo_mensaje_string(tipo_mensaje tipo){
	if(tipo == NEW_POKEMON) return "NEW_POKEMON";
	if(tipo == APPEARED_POKEMON) return "APPEARED_POKEMON";
	if(tipo == CATCH_POKEMON) return "CATCH_POKEMON";
	if(tipo == CAUGHT_POKEMON) return "CAUGHT_POKEMON";
	if(tipo == GET_POKEMON) return "GET_POKEMON";
	if(tipo == LOCALIZED_POKEMON) return "LOCALIZED";
	if(tipo == SUSCRIPTOR) return "SUSCRIPTOR";
	if(tipo == CONFIRMAR) return "CONFIRMAR";
	return "DESCONOCIDO";
}


char* consultar_config_por_string(char* path, char* key){

	t_config* config = config_create(path);

	char* cadena = string_from_format("%s",config_get_string_value(config, key));

	config_destroy(config);

	return cadena;
}


void finalizar_servidor(){
	for (int i=1; i<=6; i++){
		t_cola_mensajes* cola_mensajes = get_cola_mensajes(i);
		printf("Tipo msj %s\n", obtener_tipo_mensaje_string(i));
		destruir_cola_mensajes(cola_mensajes);
	}
	exit(2);
}


void* serializar_paquete(t_paquete* paquete, size_t bytes){
	void* magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->tipo_mensaje), sizeof(u_int32_t));
	desplazamiento += sizeof(u_int32_t);
	memcpy(magic + desplazamiento, &(paquete->id_mensaje), sizeof(u_int32_t));
	desplazamiento += sizeof(u_int32_t);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(u_int32_t));
	desplazamiento += sizeof(u_int32_t);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return magic;
}


void notificar_id_suscriptor(t_suscriptor* suscriptor, tipo_mensaje tipo_mensaje){ //envio al suscriptor un mensaje de argumentos (id, SUSCRIPTOR, cola)
	void* stream = malloc(sizeof(tipo_mensaje));
	memcpy(stream, &tipo_mensaje, sizeof(tipo_mensaje));
	t_buffer* buffer = crear_buffer(sizeof(tipo_mensaje), stream);
	t_paquete* paquete = crear_paquete(suscriptor->id, SUSCRIPTOR, buffer);

	enviar_mensaje(paquete, suscriptor->numero_socket);

	destruir_paquete(paquete);
}


u_int32_t enviar_mensaje(t_paquete* paquete, u_int32_t socket){


	size_t bytes = paquete->buffer->size + 3*sizeof(int32_t);
	void* a_enviar = serializar_paquete(paquete, bytes);

	u_int32_t envio = send(socket, a_enviar, bytes, MSG_NOSIGNAL);
	free(a_enviar);
	return envio;
}

void enviar_a_suscriptor(t_mensaje* mensaje, t_suscriptor* suscriptor){
	u_int32_t envio = enviar_mensaje(mensaje->paquete, suscriptor->numero_socket);
	printf("Se envio el mensaje de ID %d al suscriptor %d (%d)\n", mensaje->paquete->id_mensaje, suscriptor->id, envio);
	list_add(mensaje->suscriptores_enviados, suscriptor);
}

void enviar_a_suscriptores(t_mensaje* mensaje, t_list* suscriptores){
	for (u_int32_t i = 0; i < list_size(suscriptores); i++){
		t_suscriptor* suscriptor = (void*)list_get(suscriptores, i);
		enviar_a_suscriptor(mensaje, suscriptor);
		list_add(mensaje->suscriptores_enviados, suscriptor);
	}

}

void agregar_suscriptor(u_int32_t socket, t_cola_mensajes* cola_mensajes){

	t_suscriptor* suscriptor = generar_suscriptor(socket,cola_mensajes);
	printf("Asignada ID a nuevo suscriptor:%d\n", suscriptor->id);
	notificar_id_suscriptor(suscriptor, cola_mensajes->id);

	list_add(cola_mensajes->suscriptores, suscriptor);
	t_list* mensajes = cola_mensajes->mensajes;

	for(int i = 0; i < list_size(mensajes); i++){
		enviar_a_suscriptor(list_get(mensajes, i), suscriptor);
	}
}

t_suscriptor* actualizar_suscriptor(u_int32_t socket, t_cola_mensajes* cola_mensajes, u_int32_t id_suscriptor){

	t_suscriptor* suscriptor = buscar_suscriptor(cola_mensajes->suscriptores, id_suscriptor);
	suscriptor->numero_socket = socket;
	printf("Actualizado el socket del suscriptor %d: %d\n", suscriptor->id, suscriptor->numero_socket);

	t_list* mensajes = cola_mensajes->mensajes;

	for(int i = 0; i < list_size(mensajes); i++){
		t_mensaje* mensaje = list_get(mensajes, i);
		if(buscar_suscriptor(mensaje->suscriptores_confirmados, id_suscriptor)==NULL){
			enviar_a_suscriptor(mensaje, suscriptor);
		}
	}

	return suscriptor;

}

