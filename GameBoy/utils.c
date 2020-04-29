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

void enviar_mensaje(char* argv[], int socket_cliente, int tamanio){

	tipo_mensaje tipo = obtener_tipo_mensaje(argv[0]);
	t_paquete * paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = tipo;
	paquete->buffer = malloc(sizeof(t_buffer));
	int size = obtener_size(argv, tamanio) + tamanio * sizeof(int);
	printf("El size es igual a: %d\n", size);
	paquete->buffer->size = size;
	void* stream = generar_stream(argv + 1, tamanio, paquete->buffer->size);

	paquete->buffer->stream = stream;

	int size_serializado;

	void* a_enviar = serializar_paquete(paquete, &size_serializado);

	send(socket_cliente, a_enviar, size_serializado, 0);

		free(paquete->buffer);
		free(paquete);
		free(a_enviar);
}

void* generar_stream(char** argumentos, int tamanio, int size){
	int offset = 0;
	void* stream = malloc(size); 
	printf("Tamaño: %d \n", tamanio);
	for(int i = 0; i < tamanio; i++){
		int tamanio_argumento = strlen(argumentos[i])+1 ;
		printf("argumentos[%d]: %s\n", i, argumentos[i]);
		memcpy(stream + offset, &tamanio_argumento, sizeof(int));
		offset += sizeof(int);
		memcpy(stream + offset, argumentos[i], tamanio_argumento);
		offset += tamanio_argumento;
	}

	return stream;
}



int obtener_size(char* argumentos[], int tamanio){
	int size = 0;
	for(int i=0; i<tamanio; i++){
		size += (strlen(argumentos[i]) + 1);
	}
	return size;
}


/*char* recibir_mensaje(int socket_cliente){
	op_code codigo;
	int buffer_size;

	recv(socket_cliente, &codigo, sizeof(codigo), MSG_WAITALL);
	recv(socket_cliente, &buffer_size, sizeof(buffer_size), MSG_WAITALL);
	char* buffer = malloc(buffer_size);
	recv(socket_cliente, buffer, buffer_size, MSG_WAITALL);

	if(buffer[buffer_size - 1] != '\0') printf("WARN - El buffer recibido no es un string\n");

	return buffer;

}*/

void liberar_conexion(int socket_cliente){
	close(socket_cliente);
}

tipo_mensaje obtener_tipo_mensaje(char* tipo){
	if(string_equals_ignore_case(tipo,"NEW_POKEMON")) return NEW_POKEMON;
	if(string_equals_ignore_case(tipo,"APPEARED_POKEMON")) return APPEARED_POKEMON;
	if(string_equals_ignore_case(tipo,"CATCH_POKEMON")) return CATCH_POKEMON;
	if(string_equals_ignore_case(tipo,"CAUGHT_POKEMON")) return CAUGHT_POKEMON;
	if(string_equals_ignore_case(tipo,"GET_POKEMON")) return GET_POKEMON;
	return -1;
}
