/*
 * utils.c
 *
 *  Created on: 7 jun. 2020
 *      Author: utnso
 */

#include "utils.h"

/*
 * @NAME: obtener_tipo_mensaje
 * @DESC: Dado el nombre de un tipo de mensaje,
 * 		  devuelve su codigo de operacion.
 */
tipo_mensaje obtener_tipo_mensaje(char* tipo){
	tipo_mensaje tipo_mensaje;
	if(strcasecmp(tipo,"NEW_POKEMON") == 0) {tipo_mensaje = NEW_POKEMON;}
	else if(strcasecmp(tipo,"APPEARED_POKEMON") == 0) {tipo_mensaje = APPEARED_POKEMON;}
	else if(strcasecmp(tipo,"CATCH_POKEMON") == 0) {tipo_mensaje = CATCH_POKEMON;}
	else if(strcasecmp(tipo,"CAUGHT_POKEMON") == 0) {tipo_mensaje = CAUGHT_POKEMON;}
	else if(strcasecmp(tipo,"GET_POKEMON") == 0) {tipo_mensaje = GET_POKEMON;}
	else if(strcasecmp(tipo,"LOCALIZED_POKEMON") == 0) {tipo_mensaje = LOCALIZED_POKEMON;}
	else if(strcasecmp(tipo,"SUSCRIPTOR") == 0) {tipo_mensaje = SUSCRIPTOR;}
	else if(strcasecmp(tipo,"CONFIRMAR") == 0) {tipo_mensaje = CONFIRMAR;}
	return tipo_mensaje;
}

/*
 * @NAME: obtener_tipo_mensaje
 * @DESC: Dado un codigo de operacion,
 * 		  devuelve su equivalente en nombre de tipo de mensaje.
 */
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

/*
 * @NAME: recibir_cadena
 * @DESC: Dados un socket y un tamanio, recibe una cadena
 *        mandada desde el socket.
 */
char* recibir_cadena(int socket_cliente, u_int32_t* size)
{
	char * cadena;

	recv(socket_cliente, size, sizeof(u_int32_t), MSG_WAITALL);
	cadena = malloc((*size)+1);
	recv(socket_cliente, cadena, *size, MSG_WAITALL);
	cadena[(*size)] = '\0';
	return cadena;
}


/*
 * @NAME: iniciar_servidor
 * @DESC: Inicia y mantiene el servidor para la posterior
 * 		  escucha de mensajes enviados al proceso TEAM.
 */
void iniciar_servidor(void)
{
	//signal(SIGINT,liberar_todo);

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

    while (1)
    	esperar_cliente(socket_servidor);

}

/*
 * @NAME: esperar_cliente
 * @DESC: Funcion auxilar de iniciar_servidor.
 */
void esperar_cliente(int socket_servidor)
{

	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	pthread_create(&thread,NULL,(void*)serve_client,&socket_cliente);
	pthread_join(thread, NULL);

}

/*
 * @NAME: serve_client
 * @DESC: Funcion auxilar de iniciar_servidor.
 */
void recibir_mensaje(int* socket)
{

	int cod_op;

	if(recv(*socket, &cod_op, sizeof(int), MSG_WAITALL) == -1)
		cod_op = -1;
	process_request(cod_op, *socket);
}

void confirmar_recepcion(u_int32_t id_mensaje, int cliente_fd, u_int32_t id_proceso, char* mensaje) {
	char** argv = malloc(sizeof(char*) * 5);
	for (int i = 0; i < 5; i++) {
		argv[i] = string_new();
	}
	string_append(&(argv[0]), "BROKER");
	string_append(&(argv[1]), "CONFIRMAR");
	argv[2] = mensaje;
	string_append(&(argv[3]), string_itoa(id_mensaje));
	string_append(&(argv[4]), string_itoa(id_proceso));
	enviar_mensaje(argv, cliente_fd);
	printf("Mandé confirmación\n");
}

bool list_elem(char* elemento, t_list* lista){
	bool encontrado = false;
	for(int i = 0; i < list_size(lista) && !encontrado; i++){
		char* pokemon = list_get(lista, i);
		encontrado = string_equals_ignore_case(pokemon, elemento);
	}
	return encontrado;
}

DIR* verificar_existencia_de_archivo(char* pokemon) {
	char* path = string_new();
	string_append_with_format(&path, "/Files/%s", pokemon);
	char* path_pokemon = generar_nombre(path);
	DIR* directorio_pokemon = opendir(path_pokemon);
	free(path);

	if (directorio_pokemon == NULL) {
		path = string_new();
		string_append_with_format(&path, "/Files/%s", pokemon);
		char* nombre_directorio = generar_nombre(path);
		mkdir(nombre_directorio, 0777);
		generar_metadata_bin(nombre_directorio);
		free(path);
		free(nombre_directorio);
	}
	return directorio_pokemon;
}

/*
 * @NAME: process_request
 * @DESC: Funcion auxilar de iniciar_servidor.
 */
void process_request(int cod_op, int cliente_fd) {
	int size;
	u_int32_t id;
	void* msg;
	char* pokemon;
	u_int32_t size_pokemon;
	u_int32_t posicion_x;
	u_int32_t posicion_y;

	switch (cod_op) {
		case NEW_POKEMON:
			printf("Recibi un mensaje NEW_POKEMON\n");
			id = recibir_entero(cliente_fd);
			size = recibir_entero(cliente_fd);
			pokemon = recibir_cadena(cliente_fd, &size_pokemon);
			printf("Pokemon: %s\n", pokemon);
			posicion_x = recibir_entero(cliente_fd);
			posicion_y = recibir_entero(cliente_fd);
			u_int32_t cantidad = recibir_entero(cliente_fd);

			log_info(logger_gamecard, "Recibí un mensaje de tipo NEW_POKEMON y sus datos son: %s %d %d %d", pokemon, posicion_x, posicion_y, cantidad);

			confirmar_recepcion(id, cliente_fd, id_cola_new, "NEW_POKEMON");

			DIR* directorio_pokemon = verificar_existencia_de_archivo(pokemon);


			break;
		case CATCH_POKEMON:
			//printf("Recibi un mensaje CATCH_POKEMON\n");
			id = recibir_entero(cliente_fd);
			size = recibir_entero(cliente_fd);
			pokemon = recibir_cadena(cliente_fd, &size_pokemon);
			posicion_x = recibir_entero(cliente_fd);
			posicion_y = recibir_entero(cliente_fd);

			log_info(logger_gamecard, "Recibí un mensaje de tipo APPEARED_POKEMON y sus datos son: %s %d %d", pokemon, posicion_x, posicion_y);

			confirmar_recepcion(id, cliente_fd, id_cola_catch, "CATCH_POKEMON");
			break;
		case GET_POKEMON:
			//printf("Recibi un mensaje GET_POKEMON\n");
			id = recibir_entero(cliente_fd);
			size = recibir_entero(cliente_fd);
			pokemon = recibir_cadena(cliente_fd, &size_pokemon);

			log_info(logger_gamecard, "Recibí un mensaje de tipo GET_POKEMON y sus datos son: %s", pokemon);

			confirmar_recepcion(id, cliente_fd, id_cola_get, "GET_POKEMON");
			break;

		case SUSCRIPTOR:{
			u_int32_t id_cola = recibir_entero(cliente_fd);
			u_int32_t size_2 = recibir_entero(cliente_fd);
			tipo_mensaje tipo = recibir_entero(cliente_fd);
			asignar_id_cola_de_mensajes(id_cola, tipo);
			break;}
		case 0:
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
		}
}

void generar_metadata_bin(char* path){
	char* metadata = string_new();
	string_append_with_format(&metadata, "%s/Metadata.bin", path);
	printf("Path: %s\n", metadata);
	FILE* metadata_file = fopen(metadata, "w+b");
	fputs("DIRECTORY=N\n",metadata_file);
	fputs("SIZE=0\n",metadata_file);
	fputs("BLOCKS=[]\n",metadata_file);
	fputs("OPEN=Y",metadata_file);
	fclose(metadata_file);
	free(metadata);
}

void asignar_id_cola_de_mensajes(u_int32_t id_a_asignar, tipo_mensaje tipo){
	switch(tipo){
		case NEW_POKEMON:
			id_cola_new = id_a_asignar;
			break;
		case GET_POKEMON:
			id_cola_get = id_a_asignar;
			break;
		case CATCH_POKEMON:
			id_cola_catch = id_a_asignar;
			break;
		default:
			break;
	}
}

void serve_client(int* cliente_fd){
	int cod_op;

	if(recv(*cliente_fd, &cod_op, sizeof(int), MSG_WAITALL) == -1)
		cod_op = -1;

	int size;
	void* msg;
	char* pokemon;
	u_int32_t size_pokemon;
	u_int32_t posicion_x;
	u_int32_t posicion_y;
	u_int32_t id_mensaje;

	switch (cod_op) {
		case NEW_POKEMON:
			printf("Recibi un mensaje NEW_POKEMON\n");
			size = recibir_entero(*cliente_fd);
			pokemon = recibir_cadena(*cliente_fd, &size_pokemon);
			posicion_x = recibir_entero(*cliente_fd);
			posicion_y = recibir_entero(*cliente_fd);
			u_int32_t cantidad = recibir_entero(*cliente_fd);
			id_mensaje = recibir_entero(*cliente_fd);

			break;
		case CATCH_POKEMON:
			printf("Recibi un mensaje CATCH_POKEMON\n");
			size = recibir_entero(*cliente_fd);
			pokemon = recibir_cadena(*cliente_fd, &size_pokemon);
			posicion_x = recibir_entero(*cliente_fd);
			posicion_y = recibir_entero(*cliente_fd);
			id_mensaje = recibir_entero(*cliente_fd);

			break;
		case GET_POKEMON:
			printf("Recibi un mensaje GET_POKEMON\n");
			size = recibir_entero(*cliente_fd);
			printf("size: %d\n", size);
			pokemon = recibir_cadena(*cliente_fd, &size_pokemon);
			printf("size_Pokemon: %d\n", size_pokemon);
			printf("Pokemon: %s\n", pokemon);
			id_mensaje = recibir_entero(*cliente_fd);

			break;
		default:
			break;
	}
}

/*
 * @NAME: recibir_entero
 * @DESC: Dado un socket_cliente, recibe un entero desde ese socket.
 */
u_int32_t recibir_entero(int socket_cliente){
	u_int32_t entero;
	recv(socket_cliente, &entero, sizeof(u_int32_t), MSG_WAITALL);

	return entero;
}

// FUNCIONES CLIENTE

/*
 * @NAME: serializar_paquete
 * @DESC: Dado un paquete y una cantidad de bytes,
 * 		  serializa ese paquete.
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

/*
 * @NAME: crear_conexion
 * @DESC: Dados un ip y un puerto en formato de string,
 * 		  crea una conexion y devuelve el socket resultante.
 */
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

/*
 * @NAME: enviar_mensaje
 * @DESC: Dado un arreglo de argumentos en formato string,
 * 		  y un socket_cliente, manda un mensaje a ese socket.
 */
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

/*
 * @NAME: agregar_string
 * @DESC: Dados un string, un stream y un offset,
 * 		  agrega el string al stream.
 */
void agregar_string(int* offset, char* string, void** stream){
	u_int32_t longitud_nombre = strlen(string);
	memcpy((*stream) + (*offset), &longitud_nombre, sizeof(u_int32_t));
	(*offset) += sizeof(u_int32_t);
	memcpy((*stream) + (*offset), string, longitud_nombre);
	(*offset) += longitud_nombre;
}

/*
 * @NAME: agregar_entero
 * @DESC: Dados un string, un stream y un offset,
 * 		  agrega un entero en formato de string al stream.
 */
void agregar_entero(int* offset, char* string, void** stream){
	u_int32_t entero = atoi(string);
	memcpy((*stream) + (*offset), &entero, sizeof(u_int32_t));
	(*offset) += sizeof(u_int32_t);
}

/*
 * @NAME: generar_stream
 * @DESC: Dados un arreglo de argumentos en formato de string y un paquete,
 * 		  genera el stream correspondiente a ese paquete.
 */
void* generar_stream(char** argumentos, t_paquete* paquete){
	int offset = 0;
	void* stream = malloc(paquete->buffer->size);

	switch(paquete->codigo_operacion){
	    case CATCH_POKEMON:
	    	agregar_string(&offset, argumentos[2], &stream);
	    	agregar_entero(&offset, argumentos[3], &stream);
	    	agregar_entero(&offset, argumentos[4], &stream);
	    	break;
		case GET_POKEMON:
			agregar_string(&offset, argumentos[2], &stream);
			break;
		case SUSCRIPTOR:
			agregar_string(&offset, argumentos[2], &stream);
			agregar_entero(&offset, argumentos[3], &stream);
			break;
		case CONFIRMAR:{
			tipo_mensaje cod_op = obtener_tipo_mensaje(argumentos[2]);
			memcpy(stream + offset, &cod_op, sizeof(u_int32_t));
			offset += sizeof(u_int32_t);
			agregar_entero(&offset, argumentos[3], &stream);
			agregar_entero(&offset, argumentos[4], &stream);
			break;
			}
		default:
			break;
	}

	return stream;
}

/*
 * @NAME: obtener_size
 * @DESC: Dados un arreglo de argumentos en formato de string y
 * 		  codigo de operacion de un tipo de mensaje, devuelve el
 * 		  size correspondiente a ese tipo de mensaje.
 */
u_int32_t obtener_size(char* argumentos[], tipo_mensaje tipo){
	u_int32_t size = 0;
	switch(tipo){
		case CATCH_POKEMON:
			size = sizeof(u_int32_t) * 3 + strlen(argumentos[2]);
			break;
		case GET_POKEMON:
			size = sizeof(u_int32_t) + strlen(argumentos[2]);
			break;
		case SUSCRIPTOR:
			size = sizeof(u_int32_t) * 2 + strlen(argumentos[2]);
			break;
		case CONFIRMAR:
			size = sizeof(u_int32_t) * 3;
			break;
		default:
			break;
	}

	return size;
}

/*
 * @NAME: liberar_conexion
 * @DESC: Libera la conexion con un socket_cliente pasado por parametro.
 */
void liberar_conexion(u_int32_t socket_cliente){
	close(socket_cliente);
}

char* generar_nombre(char* parametro){
	char* nombre = string_new();
	string_append(&nombre, config_gamecard->punto_montaje_tallgrass);
	string_append(&nombre, parametro);
	return nombre;
}

