/*
 * utils.c
 *
 *  Created on: 6 may. 2020
 *      Author: utnso
 */

#include "utils.h"

// FUNCIONES SERVIDOR

tipo_mensaje obtener_tipo_mensaje(char* tipo){
	tipo_mensaje tipo_mensaje;
	if(strcasecmp(tipo,"APPEARED_POKEMON") == 0) {tipo_mensaje = APPEARED_POKEMON;}
	else if(strcasecmp(tipo,"GET_POKEMON") == 0) {tipo_mensaje = GET_POKEMON;}
	return tipo_mensaje;
}

char* obtener_tipo_mensaje_string(tipo_mensaje tipo){
	if(tipo == APPEARED_POKEMON) return "APPEARED_POKEMON";
	return "DESCONOCIDO";
}

void* recibir_cadena(int socket_cliente, int* size)
{
	void * cadena;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	cadena = malloc(*size);
	recv(socket_cliente, cadena, *size, MSG_WAITALL);

	return cadena;
}

t_appeared_pokemon* iniciar_servidor(void)
{

	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(IP, PUERTO, &hints, &servinfo);

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

    t_parametros* parametros = malloc(sizeof(t_parametros));

    parametros->appeared_pokemon = NULL;

    while(parametros->appeared_pokemon == NULL){
    	esperar_cliente(socket_servidor, parametros);
    }

    t_appeared_pokemon* appeared_pokemon = parametros->appeared_pokemon;

    free(parametros);

    return appeared_pokemon;

}

void esperar_cliente(int socket_servidor, t_parametros* parametros)
{
	struct sockaddr_in dir_cliente;

	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	parametros->socket_cliente = &socket_cliente;

	pthread_create(&thread,NULL,(void*)serve_client, parametros);
	pthread_join(thread, NULL);

	//printf("p: %s\n", parametros->appeared_pokemon->pokemon);

}

void serve_client(t_parametros* parametros) // int* socket)
{
	int cod_op;
	if(recv(*(parametros->socket_cliente), &cod_op, sizeof(int), MSG_WAITALL) == -1)
		cod_op = -1;
	process_request(cod_op, parametros);//*socket);

}



void process_request(int cod_op, t_parametros* parametros){// int cliente_fd) {
	int size;
	void* msg;
		switch (cod_op) {
		case APPEARED_POKEMON:
			printf("Recibí un mensaje de tipo APPEARED_POKEMON\n");

			t_appeared_pokemon* appeared_pokemon = malloc(sizeof(t_appeared_pokemon));

			size = recibir_entero(*(parametros->socket_cliente));

			appeared_pokemon->pokemon = recibir_cadena(*(parametros->socket_cliente), &(appeared_pokemon->size_pokemon));
			appeared_pokemon->posicion_x = recibir_entero(*(parametros->socket_cliente));
			appeared_pokemon->posicion_y = recibir_entero(*(parametros->socket_cliente));

			parametros->appeared_pokemon = appeared_pokemon;

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

// FUNCIONES CLIENTE

void* serializar_paquete(t_paquete* paquete, u_int32_t *bytes){
	u_int32_t size_serializado = sizeof(paquete->codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;
	void* buffer = malloc(size_serializado);

	u_int32_t bytes_escritos = 0;

	memcpy(buffer + bytes_escritos, &(paquete->codigo_operacion), sizeof(paquete->codigo_operacion));
	bytes_escritos += sizeof(paquete->codigo_operacion);

	memcpy(buffer + bytes_escritos, &(paquete->buffer->size), sizeof(paquete->buffer->size));
	bytes_escritos += sizeof(paquete->buffer->size);

	memcpy(buffer + bytes_escritos, paquete->buffer->stream, paquete->buffer->size);
	bytes_escritos += paquete->buffer->size;

	*bytes = size_serializado;

	return buffer;
}

int crear_conexion(char *ip, char* puerto){
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	u_int32_t socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		printf("error");

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(char* argv[], u_int32_t socket_cliente){
	tipo_mensaje tipo = obtener_tipo_mensaje(argv[1]);
	t_paquete * paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = tipo;
	paquete->buffer = malloc(sizeof(t_buffer));
	u_int32_t size = obtener_size(argv, tipo);
	paquete->buffer->size = size;
	void* stream = generar_stream(argv, paquete);

	paquete->buffer->stream = stream;

	u_int32_t size_serializado;

	void* a_enviar = serializar_paquete(paquete, &size_serializado);

	send(socket_cliente, a_enviar, size_serializado, 0);

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	free(a_enviar);
}

void agregar_string(int* offset, char* string, void** stream){
	u_int32_t longitud_nombre = strlen(string) + 1;
	memcpy((*stream) + (*offset), &longitud_nombre, sizeof(u_int32_t));
	(*offset) += sizeof(u_int32_t);
	memcpy((*stream) + (*offset), string, longitud_nombre);
	(*offset) += longitud_nombre;
}

void agregar_entero(int* offset, char* string, void** stream){
	u_int32_t entero = atoi(string);
	memcpy((*stream) + (*offset), &entero, sizeof(u_int32_t));
	(*offset) += sizeof(u_int32_t);
}

void* generar_stream(char** argumentos, t_paquete* paquete){
	int offset = 0;
	void* stream = malloc(paquete->buffer->size);

	switch(paquete->codigo_operacion){
		case GET_POKEMON:
			agregar_string(&offset, argumentos[2], &stream);
			break;
		default:
			break;
	}

	return stream;
}

u_int32_t obtener_size(char* argumentos[], tipo_mensaje tipo){
	u_int32_t size = 0;
	switch(tipo){
		case GET_POKEMON:
			size = sizeof(u_int32_t) + strlen(argumentos[2]) + 1;
			break;
		default:
			break;
	}

	return size;
}

void liberar_conexion(u_int32_t socket_cliente){
	close(socket_cliente);
}
