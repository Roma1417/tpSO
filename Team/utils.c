/*
 * utils.c
 *
 *  Created on: 6 may. 2020
 *      Author: utnso
 */

#include "utils.h"

// FUNCIONES SERVIDOR

/*
 * @NAME: obtener_tipo_mensaje
 * @DESC: Dado el nombre de un tipo de mensaje,
 * 		  devuelve su codigo de operacion.
 */
tipo_mensaje obtener_tipo_mensaje(char* tipo){
	tipo_mensaje tipo_mensaje;
	if(strcasecmp(tipo,"APPEARED_POKEMON") == 0) {tipo_mensaje = APPEARED_POKEMON;}
	else if(strcasecmp(tipo,"CATCH_POKEMON") == 0) {tipo_mensaje = CATCH_POKEMON;}
	else if(strcasecmp(tipo,"GET_POKEMON") == 0) {tipo_mensaje = GET_POKEMON;}
	else if(strcasecmp(tipo,"SUSCRIPTOR") == 0) {tipo_mensaje = SUSCRIPTOR;}
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
void* recibir_cadena(int socket_cliente, int* size)
{
	void * cadena;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	cadena = malloc(*size);
	recv(socket_cliente, cadena, *size, MSG_WAITALL);

	return cadena;
}

/*void liberar_todo(int n){
	printf("Lo intento\n");
	list_clean(objetivo_global);
	shutdown(socket_servidor, SHUT_RDWR);
}*/

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

    // Falta ver la condicion del while
    // Deberia ser algo como: Mientras no se cumpla el objetivo global...

    // while (no se cumple objetivo global) -> funca servidor
    // se cumplio entonces -> sem_servidor = 0;

    while (!quedan_pokemons_por_atrapar())
    	esperar_cliente(socket_servidor);

}

/*
 * @NAME: quedan_pokemons_por_atrapar
 * @DESC: Se fija si aun quedan pokemons por atrapar. Esto se produce cuando
 * 		  ya no quedan mas objetivos en objetivo global.
 */
bool quedan_pokemons_por_atrapar(){
	bool objetivo_cumplido = true;

	for(int i = 0; i < list_size(objetivo_global) && objetivo_cumplido; i++){
		t_list* objetivos = list_get(objetivo_global, i);
		objetivo_cumplido = list_is_empty(objetivos);
	}

	return objetivo_cumplido;
}

/*
 * @NAME: esperar_cliente
 * @DESC: Funcion auxilar de iniciar_servidor.
 */
void esperar_cliente(int socket_servidor)
{

	struct sockaddr_in dir_cliente; //¿Puedo sacar la ip y el ?
	int tam_direccion = sizeof(struct sockaddr_in);
	//socklen_t tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	pthread_create(&thread,NULL,(void*)serve_client,&socket_cliente);
	pthread_join(thread, NULL);

}

/*
 * @NAME: serve_client
 * @DESC: Funcion auxilar de iniciar_servidor.
 */
void serve_client(int* socket)
{
	int cod_op;
	if(recv(*socket, &cod_op, sizeof(int), MSG_WAITALL) == -1)
		cod_op = -1;
	process_request(cod_op, *socket);
}

/*
 * @NAME: sigue_en_falta_especie
 * @DESC: Dado un nombre de una especie de pokemons, se fija si
 * 		  aun se sigue esperando que aparezcan pokemons de esa misma
 * 		  especie. Si es asi resta 1 a la cantidad total de faltantes.
 */
bool sigue_en_falta_especie(char* pokemon){
	bool en_falta = false;
	for (int i = 0; i < list_size(especies_requeridas) && !en_falta; i++){
		t_especie* especie = list_get(especies_requeridas, i);
		en_falta = string_equals_ignore_case(especie->nombre, pokemon) && especie->cantidad > 0;
		if (en_falta) especie->cantidad--;
	}
	return en_falta;
}

/*
 * @NAME: process_request
 * @DESC: Funcion auxilar de iniciar_servidor.
 */
void process_request(int cod_op, int cliente_fd) {
	int size;
	void* msg;
		switch (cod_op) {
		case APPEARED_POKEMON:
			printf("Recibí un mensaje de tipo APPEARED_POKEMON\n");

			t_appeared_pokemon* appeared_pokemon = appeared_pokemon_create();

			size = recibir_entero(cliente_fd);

			char* cadena = recibir_cadena(cliente_fd, &(appeared_pokemon->size_pokemon));

			cambiar_nombre_pokemon(appeared_pokemon, cadena);

			u_int32_t x = recibir_entero(cliente_fd);
			u_int32_t y = recibir_entero(cliente_fd);

			t_posicion* posicion = posicion_create(x,y);

			cambiar_posicion(appeared_pokemon,posicion);

			// Posible uso de semaforos en esta parte

			if (list_elem(appeared_pokemon->pokemon, objetivo_global)
					&& sigue_en_falta_especie(appeared_pokemon->pokemon)){
				queue_push(appeared_pokemons, appeared_pokemon);
				sem_post(&sem_appeared_pokemon);
			} else appeared_pokemon_destroy(appeared_pokemon);

			break;

		case SUSCRIPTOR:{
			printf("Buenass\n");
			u_int32_t cola_de_mensajes_size = recibir_entero(cliente_fd);
			u_int32_t id_cola = recibir_entero(cliente_fd);
			u_int32_t size_2 = recibir_entero(cliente_fd);
			tipo_mensaje tipo = recibir_entero(cliente_fd);
			printf("Tipo: %d\n", tipo);
			asignar_id_cola_de_mensajes(id_cola, tipo);

			break;}
		case 0:
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
		}
}

void asignar_id_cola_de_mensajes(u_int32_t id_a_asignar, tipo_mensaje tipo){
	switch(tipo){
		case APPEARED_POKEMON:
			id_cola_appeared = id_a_asignar;
			break;
		case LOCALIZED_POKEMON:
			id_cola_localized = id_a_asignar;
			break;
		case CAUGHT_POKEMON:
			id_cola_caught = id_a_asignar;
			break;
	}
}

/*
 * @NAME: recibir_entero
 * @DESC: Dado un socket_cliente, recibe un entero desde ese socket.
 */
int recibir_entero(int socket_cliente){
	int entero;

	recv(socket_cliente, &entero, sizeof(int), MSG_DONTWAIT);

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

	int estado = send(socket_cliente, a_enviar, size_serializado, 0);

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
	u_int32_t longitud_nombre = strlen(string) + 1;
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
			size = sizeof(u_int32_t) * 3 + strlen(argumentos[2]) + 1;
			break;
		case GET_POKEMON:
			size = sizeof(u_int32_t) + strlen(argumentos[2]) + 1;
			break;
		case SUSCRIPTOR:
			size = sizeof(u_int32_t) * 2 + strlen(argumentos[2]) + 1;
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
