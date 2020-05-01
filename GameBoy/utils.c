#include "utils.h"

/*
 * Recibe un paquete a serializar, y un puntero a un u_int32_t en el que dejar
 * el tamaño del stream de bytes serializados que devuelve
 */
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

void enviar_mensaje(char* argv[], u_int32_t socket_cliente, u_int32_t tamanio){

	tipo_mensaje tipo = obtener_tipo_mensaje(argv[0]);
	t_paquete * paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = tipo;
	paquete->buffer = malloc(sizeof(t_buffer));
	u_int32_t size = obtener_size(argv, tamanio, tipo);
	printf("El size es igual a: %d\n", size);
	paquete->buffer->size = size;
	void* stream = generar_stream(argv + 1, tamanio, paquete);

	paquete->buffer->stream = stream;

	u_int32_t size_serializado;

	void* a_enviar = serializar_paquete(paquete, &size_serializado);

	send(socket_cliente, a_enviar, size_serializado, 0);

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

void* generar_stream(char** argumentos, u_int32_t tamanio, t_paquete* paquete){
	int offset = 0;
	void* stream = malloc(paquete->buffer->size); 
	
	switch(paquete->codigo_operacion){
		case NEW_POKEMON:{
			agregar_string(&offset, argumentos[0], &stream);
			for(u_int32_t i=1; i<4; i++){
				agregar_entero(&offset, argumentos[i], &stream);
			}
			break;
		}	
		case APPEARED_POKEMON:
			agregar_string(&offset, argumentos[0], &stream);
			for(u_int32_t i=1; i<4; i++){
				agregar_entero(&offset, argumentos[i], &stream);
			}
			break;
		case CATCH_POKEMON:
			agregar_string(&offset, argumentos[0], &stream);
			for(u_int32_t i=1; i<3; i++){
				agregar_entero(&offset, argumentos[i], &stream);
			}
			break;
		case CAUGHT_POKEMON:
			for(u_int32_t i=0; i<2; i++){
				agregar_entero(&offset, argumentos[i], &stream);
			}
			break;
		case GET_POKEMON:
			agregar_string(&offset, argumentos[0], &stream);
			break;
		default:
			break;
	}

	return stream;
}

u_int32_t obtener_size(char* argumentos[], u_int32_t tamanio, tipo_mensaje tipo){
	u_int32_t size = 0;
	
	switch(tipo){
		case NEW_POKEMON:
			size = sizeof(u_int32_t) * 4 + strlen(argumentos[1]) + 1;
			break;
		case APPEARED_POKEMON:
			size = sizeof(u_int32_t) * 4 + strlen(argumentos[1]) + 1;
			break;
		case CATCH_POKEMON:
			size = sizeof(u_int32_t) * 3 + strlen(argumentos[1]) + 1;
			break;
		case CAUGHT_POKEMON:
			size = sizeof(u_int32_t) * 2; 
			break;
		case GET_POKEMON:
			size = sizeof(u_int32_t) + strlen(argumentos[1]) + 1;
			break;
		default:
			break;
	}
	
	return size;
}


void liberar_conexion(u_int32_t socket_cliente){
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
