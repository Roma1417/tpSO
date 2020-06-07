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
	int i_max = 0;
	void* stream = malloc(paquete->buffer->size); 
	
	switch(paquete->codigo_operacion){
		case NEW_POKEMON:{
			agregar_string(&offset, argumentos[2], &stream);
			
			if(string_equals_ignore_case(argumentos[0],"BROKER")) i_max=6;
			else i_max = 7;

			for(int i=3; i<i_max; i++){
				agregar_entero(&offset, argumentos[i], &stream);
			}
			break;
		}	
		case APPEARED_POKEMON:
			agregar_string(&offset, argumentos[2], &stream);

			if(string_equals_ignore_case(argumentos[0],"BROKER")) i_max=6;
			else i_max = 5;

			for(int i=3; i<i_max; i++){
				agregar_entero(&offset, argumentos[i], &stream);
			}
			break;
		case CATCH_POKEMON:
			agregar_string(&offset, argumentos[2], &stream);

			if(string_equals_ignore_case(argumentos[0],"BROKER")) i_max=5;
			else i_max = 6;

			for(int i=3; i<i_max; i++){
				agregar_entero(&offset, argumentos[i], &stream);
			}
			break;
		case CAUGHT_POKEMON:
			for(int i=2; i<4; i++){
				agregar_entero(&offset, argumentos[i], &stream);
			}
			break;
		case GET_POKEMON:
			agregar_string(&offset, argumentos[2], &stream);

			if(string_equals_ignore_case(argumentos[0],"BROKER")) i_max=3;
				else i_max = 4;

			for(int i=3; i<i_max; i++){
				agregar_entero(&offset, argumentos[i], &stream);
			}
			break;
		case SUSCRIPTOR:{
			agregar_string(&offset, argumentos[2], &stream);
			agregar_entero(&offset, "0", &stream); // Modificamos el formato del mensaje SUSCRIPTOR
			break;
			}
		default:
			break;
	}

	return stream;
}

u_int32_t obtener_size(char* argumentos[], tipo_mensaje tipo){
	u_int32_t size = 0;
	switch(tipo){
		case NEW_POKEMON:
			if(string_equals_ignore_case(argumentos[0],"BROKER"))
				size = sizeof(u_int32_t) * 4 + strlen(argumentos[2]) + 1;
			else size = sizeof(u_int32_t) * 5 + strlen(argumentos[2]) + 1;
			break;
		case APPEARED_POKEMON:
			if(string_equals_ignore_case(argumentos[0],"BROKER"))
				size = sizeof(u_int32_t) * 4 + strlen(argumentos[2]) + 1;
			else size = sizeof(u_int32_t) * 3 + strlen(argumentos[2]) + 1;
			break;
		case CATCH_POKEMON:
			if(string_equals_ignore_case(argumentos[0],"BROKER"))
				size = sizeof(u_int32_t) * 3 + strlen(argumentos[2]) + 1;
			else size = sizeof(u_int32_t) * 4 + strlen(argumentos[2]) + 1;
			break;
		case CAUGHT_POKEMON:
			size = sizeof(u_int32_t) * 2; 
			break;
		case GET_POKEMON:
			if(string_equals_ignore_case(argumentos[0],"BROKER"))
				size = sizeof(u_int32_t) + strlen(argumentos[2]) + 1;
			else size = sizeof(u_int32_t) * 2 + strlen(argumentos[2]) + 1;
			break;
		case SUSCRIPTOR:
			size = sizeof(u_int32_t) *2 + strlen(argumentos[2]) +1;
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
	tipo_mensaje tipo_mensaje;
	if(strcasecmp(tipo,"NEW_POKEMON") == 0){tipo_mensaje = NEW_POKEMON;}
	else if(strcasecmp(tipo,"APPEARED_POKEMON") == 0) {tipo_mensaje = APPEARED_POKEMON;}
	else if(strcasecmp(tipo,"CATCH_POKEMON") == 0) {tipo_mensaje = CATCH_POKEMON;}
	else if(strcasecmp(tipo,"CAUGHT_POKEMON") == 0) {tipo_mensaje = CAUGHT_POKEMON;}
	else if(strcasecmp(tipo,"GET_POKEMON") == 0) {tipo_mensaje = GET_POKEMON;}
	else if(strcasecmp(tipo,"SUSCRIPTOR") == 0) {tipo_mensaje = SUSCRIPTOR;}
	else tipo_mensaje = DESCONOCIDO;
	return tipo_mensaje;
}

char* obtener_tipo_mensaje_string(tipo_mensaje tipo){
	if(tipo == NEW_POKEMON) return "NEW_POKEMON";
	if(tipo == APPEARED_POKEMON) return "APPEARED_POKEMON";
	if(tipo == CATCH_POKEMON) return "CATCH_POKEMON";
	if(tipo == CAUGHT_POKEMON) return "CAUGHT_POKEMON";
	if(tipo == GET_POKEMON) return "GET_POKEMON";
	if(tipo == LOCALIZED_POKEMON) return "LOCALIZED";
	if(tipo == SUSCRIPTOR) return "SUSCRIPTOR";
	return "DESCONOCIDO";
}

void validar_argumentos(char** argumentos, int cantidad){
	tipo_mensaje tipo = obtener_tipo_mensaje(argumentos[2]);
	if((strcasecmp(argumentos[1],"BROKER") == 0) && (tipo == DESCONOCIDO)){
		printf("No se puede mandar el mensaje %s al proceso Broker\n", argumentos[2]);
		exit(1);
	}
	else if((strcasecmp(argumentos[1],"TEAM") == 0) && (tipo != APPEARED_POKEMON)){
		printf("No se puede mandar el mensaje %s al proceso Team\n", argumentos[2]);
		exit(1);
	}
	else if((strcasecmp(argumentos[1],"GAMECARD") == 0) && ((tipo != NEW_POKEMON) && (tipo != CATCH_POKEMON) && (tipo != GET_POKEMON))){
		printf("No se puede mandar el mensaje %s al proceso GameCard\n", argumentos[2]);
		exit(1);
	}
}

void* recibir_cadena(int socket_cliente, int* size)
{
	void * cadena;

	recv(socket_cliente, size, sizeof(int), MSG_DONTWAIT);
	cadena = malloc(*size);
	recv(socket_cliente, cadena, *size, MSG_DONTWAIT);

	return cadena;
}

u_int32_t recibir_entero(int socket_cliente){
	int entero;
	recv(socket_cliente, &entero, sizeof(int), MSG_DONTWAIT);

	return entero;
}

void recibir_mensaje(int* conexion){
	int cod_op;
	if(recv(*conexion, &cod_op, sizeof(int), MSG_DONTWAIT) == -1)
		cod_op = -1;
	process_request(cod_op, *conexion);
}
void process_request(int cod_op, int cliente_fd) {

	switch (cod_op) {
		case NEW_POKEMON:
		case APPEARED_POKEMON:
		case CATCH_POKEMON:
		case CAUGHT_POKEMON:
		case GET_POKEMON:
		case LOCALIZED_POKEMON:
			printf("Recibí un mensaje de tipo %s\n", (char*) obtener_tipo_mensaje_string(cod_op));

			int32_t size;
			void* stream = (void*)recibir_cadena(cliente_fd, &size);
			printf("Parametro cadena %s\n",(char*) (stream+4));

			break;

		case SUSCRIPTOR:
			printf("Recibí un mensaje de tipo %s\n", (char*) obtener_tipo_mensaje_string(cod_op));
			u_int32_t id_cola = recibir_entero(cliente_fd);
			u_int32_t size_2 = recibir_entero(cliente_fd);
			tipo_mensaje tipo = recibir_entero(cliente_fd);
			break;
		default:
			break;

		}
}



