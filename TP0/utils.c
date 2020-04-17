/*
 * conexiones.c
 *
 *  Created on: 2 mar. 2019
 *      Author: utnso
 */

#include "utils.h"

/*
 * Recibe un paquete a serializar, y un puntero a un int en el que dejar
 * el tamaño del stream de bytes serializados que devuelve
 */
void* serializar_paquete(t_paquete* paquete, int *bytes){
	int size_serializado = sizeof(paquete->codigo_operacion) + sizeof(paquete->buffer->size) + paquete->buffer->size;
	void* buffer = malloc(size_serializado);

	int bytes_escritos = 0;

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

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		printf("error");

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente){

	t_paquete * paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = mensaje;
	int size_serializado;

	void* a_enviar = serializar_paquete(paquete, &size_serializado);
	send(socket_cliente, a_enviar, size_serializado, 0);

	free(paquete->buffer);
	free(paquete);
	free(a_enviar);
}

char* recibir_mensaje(int socket_cliente){
	op_code codigo;
	int buffer_size;

	recv(socket_cliente, &codigo, sizeof(codigo), MSG_WAITALL);
	recv(socket_cliente, &buffer_size, sizeof(buffer_size), MSG_WAITALL);
	char* buffer = malloc(buffer_size);
	recv(socket_cliente, buffer, buffer_size, MSG_WAITALL);

	if(buffer[buffer_size - 1] != '\0') printf("WARN - El buffer recibido no es un string\n");

	return buffer;

}

void liberar_conexion(int socket_cliente){
	close(socket_cliente);
}
