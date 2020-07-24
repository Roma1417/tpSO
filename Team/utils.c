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
tipo_mensaje obtener_tipo_mensaje(char* tipo) {
	tipo_mensaje tipo_mensaje;
	if (strcasecmp(tipo, "APPEARED_POKEMON") == 0) {
		tipo_mensaje = APPEARED_POKEMON;
	}
	else if (strcasecmp(tipo, "CATCH_POKEMON") == 0) {
		tipo_mensaje = CATCH_POKEMON;
	}
	else if (strcasecmp(tipo, "CAUGHT_POKEMON") == 0) {
		tipo_mensaje = CAUGHT_POKEMON;
	}
	else if (strcasecmp(tipo, "GET_POKEMON") == 0) {
		tipo_mensaje = GET_POKEMON;
	}
	else if (strcasecmp(tipo, "LOCALIZED_POKEMON") == 0) {
		tipo_mensaje = LOCALIZED_POKEMON;
	}
	else if (strcasecmp(tipo, "SUSCRIPTOR") == 0) {
		tipo_mensaje = SUSCRIPTOR;
	}
	else if (strcasecmp(tipo, "CONFIRMAR") == 0) {
		tipo_mensaje = CONFIRMAR;
	}
	return tipo_mensaje;
}

/*
 * @NAME: obtener_tipo_mensaje
 * @DESC: Dado un codigo de operacion,
 * 		  devuelve su equivalente en nombre de tipo de mensaje.
 */
char* obtener_tipo_mensaje_string(tipo_mensaje tipo) {
	if (tipo == NEW_POKEMON) return "NEW_POKEMON";
	if (tipo == APPEARED_POKEMON) return "APPEARED_POKEMON";
	if (tipo == CATCH_POKEMON) return "CATCH_POKEMON";
	if (tipo == CAUGHT_POKEMON) return "CAUGHT_POKEMON";
	if (tipo == GET_POKEMON) return "GET_POKEMON";
	if (tipo == LOCALIZED_POKEMON) return "LOCALIZED_POKEMON";
	if (tipo == SUSCRIPTOR) return "SUSCRIPTOR";
	if (tipo == CONFIRMAR) return "CONFIRMAR";
	return "DESCONOCIDO";
}

/*
 * @NAME: recibir_cadena
 * @DESC: Dados un socket y un tamanio, recibe una cadena
 *        mandada desde el socket.
 */
char* recibir_cadena(int socket_cliente, u_int32_t* size) {
	char * cadena;

	recv(socket_cliente, size, sizeof(u_int32_t), MSG_WAITALL);
	cadena = malloc((*size) + 1);
	recv(socket_cliente, cadena, *size, MSG_WAITALL);
	cadena[(*size)] = 0;

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
void iniciar_servidor(void) {

	//signal(SIGINT,liberar_todo);

	int socket_servidor;

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(IP, PUERTO, &hints, &servinfo);

	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) continue;

		if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
			close(socket_servidor);
			continue;
		}
		break;
	}

/*	int flags = fcntl(socket_servidor, F_GETFL, 0);
	fcntl(socket_servidor, F_SETFL, flags | O_NONBLOCK);*/

	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);

	// Falta ver la condicion del while
	// Deberia ser algo como: Mientras no se cumpla el objetivo global...

	// while (no se cumple objetivo global) -> funca servidor
	// se cumplio entonces -> sem_servidor = 0;

	// Cambiar por algo mas feliz (para Josi)
	while (!pokemons_objetivo_fueron_atrapados()){
		esperar_cliente(socket_servidor);
	}

}

/*
 * @NAME: pokemons_objetivo_fueron_atrapados
 * @DESC: Se fija si aun quedan pokemons por atrapar. Esto se produce cuando
 * 		  ya no quedan mas objetivos en objetivo global.
 */
bool pokemons_objetivo_fueron_atrapados() {
	bool objetivo_cumplido = true;

	t_list* objetivos;
	for (int i = 0; i < list_size(objetivo_global) && objetivo_cumplido; i++) {
		objetivos = list_get(objetivo_global, i);

		objetivo_cumplido = list_is_empty(objetivos);
	}


	return objetivo_cumplido;
}

/*
 * @NAME: esperar_cliente
 * @DESC: Funcion auxilar de iniciar_servidor.
 */
void esperar_cliente(int socket_servidor) {

	struct sockaddr_in dir_cliente; //¿Puedo sacar la ip y el ?
	int tam_direccion = sizeof(struct sockaddr_in);
	//socklen_t tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	pthread_create(&thread, NULL, (void*) serve_client, &socket_cliente);

	pthread_join(thread, NULL);

}

/*
 * @NAME: serve_client
 * @DESC: Funcion auxilar de iniciar_servidor.
 */
void recibir_mensaje(int* socket) {
	int cod_op;
	if (recv(*socket, &cod_op, sizeof(int), MSG_WAITALL) == -1) cod_op = -1;
	process_request(cod_op, *socket);
}

/*
 * @NAME: sigue_en_falta_especie
 * @DESC: Dado un nombre de una especie de pokemons, se fija si
 * 		  aun se sigue esperando que aparezcan pokemons de esa misma
 * 		  especie. Si es asi resta 1 a la cantidad total de faltantes.
 */
bool sigue_en_falta_especie(char* pokemon) {
	bool en_falta = false;
	for (int i = 0; i < list_size(especies_requeridas) && !en_falta; i++) {
		t_especie* especie = list_get(especies_requeridas, i);
		en_falta = string_equals_ignore_case(especie->nombre, pokemon) && especie->cantidad > 0;
		if (en_falta) especie->cantidad--;
	}
	return en_falta;
}

void recibir_pokemon(char* pokemon){
	bool se_encontro = false;
	t_especie* especie;
	for(int i=0; i<list_size(especies_requeridas) && !se_encontro; i++){
		especie = list_get(especies_requeridas, i);
		if(string_equals_ignore_case(pokemon, especie->nombre)){
			se_encontro = true;
			especie->fueRecibida = true;
		}
	}
}

void serve_client(int* socket) {
	int cod_op;

	if (recv(*socket, &cod_op, sizeof(int), MSG_WAITALL) == -1)	cod_op = -1;

	//printf("El codigo de operacion es: %d\n", cod_op);

	/*if(cod_op == -1) pthread_exit(NULL);*/

	if (cod_op == APPEARED_POKEMON) {
		t_appeared_pokemon* appeared_pokemon = appeared_pokemon_create();
		recibir_entero(*socket);

		char* cadena = recibir_cadena(*socket, &(appeared_pokemon->size_pokemon));

		//*(cadena+appeared_pokemon->size_pokemon)=0; //Es un toque inestable, pero funciona
		cambiar_nombre_pokemon(appeared_pokemon, cadena);
		u_int32_t x = recibir_entero(*socket);
		u_int32_t y = recibir_entero(*socket);

		t_posicion* posicion = posicion_create(x, y);
		cambiar_posicion(appeared_pokemon, posicion);

		// Posible uso de semaforos en esta parte

		log_info(logger_team, "Recibí un mensaje de tipo APPEARED_POKEMON y sus datos son: %s %d %d\n", cadena, x, y);

		if (list_elem(appeared_pokemon->pokemon, objetivo_global) && sigue_en_falta_especie(appeared_pokemon->pokemon)) {
			recibir_pokemon(appeared_pokemon->pokemon);
			if(get_algoritmo_planificacion() == SJFCD) sem_wait(&puede_ser_pusheado);

			queue_push(appeared_pokemons, appeared_pokemon);

			sem_post(&sem_appeared_pokemon);
		}
		else appeared_pokemon_destroy(appeared_pokemon);


	}
}


void confirmar_recepcion(u_int32_t id_mensaje, u_int32_t id_proceso, char* mensaje) {
	int cliente_fd = crear_conexion(config_team->ip_broker, config_team->puerto_broker);
	char** argv = malloc(sizeof(char*) * 5);

	for (int i = 0; i < 5; i++) if (i != 2) argv[i] = string_new();

	string_append(&(argv[0]), "BROKER");
	string_append(&(argv[1]), "CONFIRMAR");
	argv[2] = mensaje;
	char* auxiliar = string_itoa(id_mensaje);
	string_append(&(argv[3]), auxiliar);
	free(auxiliar);
	auxiliar = string_itoa(id_proceso);
	string_append(&(argv[4]), auxiliar);
	free(auxiliar);
	enviar_mensaje(argv, cliente_fd);

	for (int i = 0; i < 5; i++) if (i != 2) free(argv[i]);
	free(argv);

	liberar_conexion(cliente_fd);
}

char* obtener_resultado(u_int32_t resultado) {
	char* string = string_new();
	if (resultado) string_append(&string, "OK");
	else string_append(&string, "FAIL");
	return string;
}

t_appeared_pokemon* crear_localized_pokemon(char* pokemon, uint32_t pos_x, uint32_t pos_y){
	t_appeared_pokemon* localized_pokemon = malloc(sizeof(t_appeared_pokemon));
	localized_pokemon->pokemon = pokemon;

	t_posicion* posicion = posicion_create(pos_x, pos_y);
	localized_pokemon->posicion = posicion;

	return localized_pokemon;
}

bool ya_recibio_especie(char* pokemon){
	t_especie* especie;
	for(int i=0; i<list_size(especies_requeridas); i++){
		especie = list_get(especies_requeridas, i);
		if(string_equals_ignore_case(pokemon, especie->nombre)) return especie->fueRecibida;
	}
	return false;
}

bool esta_en_ids_get(uint32_t id){
	bool encontrado = false;
	for(int i=0;i<list_size(ids_gets) && !encontrado; i++){
		uint32_t id_encontrado = (uint32_t) list_get(ids_gets, i);
		encontrado = (id_encontrado == id);
	}
	return encontrado;
}

/*
 * @NAME: process_request
 * @DESC: Funcion auxilar de iniciar_servidor.
 */
void process_request(int cod_op, int cliente_fd) {
	int size;
	void* msg;
	u_int32_t id;
	u_int32_t id_correlativo;
	//printf("Cod op: %d\n", cod_op);
	if(cod_op < 0) 	sleep(config_team->tiempo_reconexion);

	switch (cod_op) {
		case APPEARED_POKEMON: {
			id = recibir_entero(cliente_fd);
			t_appeared_pokemon* appeared_pokemon = appeared_pokemon_create();
			recibir_entero(cliente_fd);
			id_correlativo = recibir_entero(cliente_fd);
			char* cadena = recibir_cadena(cliente_fd, &(appeared_pokemon->size_pokemon));
			cambiar_nombre_pokemon(appeared_pokemon, cadena);
			u_int32_t x = recibir_entero(cliente_fd);
			u_int32_t y = recibir_entero(cliente_fd);
			t_posicion* posicion = posicion_create(x, y);
			cambiar_posicion(appeared_pokemon, posicion);

			// Posible uso de semaforos en esta parte

			log_info(logger_team, "Recibí un mensaje de tipo APPEARED_POKEMON y sus datos son: %s %d %d %d\n", cadena, x, y, id_correlativo);

			if (list_elem(appeared_pokemon->pokemon, objetivo_global) && sigue_en_falta_especie(appeared_pokemon->pokemon)) {
				recibir_pokemon(appeared_pokemon->pokemon);
				if(get_algoritmo_planificacion() == SJFCD) sem_wait(&puede_ser_pusheado);
				queue_push(appeared_pokemons, appeared_pokemon);
				sem_post(&sem_appeared_pokemon);
			}
			else appeared_pokemon_destroy(appeared_pokemon);

			confirmar_recepcion(id, id_cola_appeared, "APPEARED_POKEMON");

			break;
		}
		case CATCH_POKEMON:
			break;
		case CAUGHT_POKEMON: {

			id = recibir_entero(cliente_fd);

			size = recibir_entero(cliente_fd);

			id_correlativo = recibir_entero(cliente_fd);

			//u_int32_t id_mensaje = recibir_entero(cliente_fd);

			u_int32_t resultado = recibir_entero(cliente_fd);

			char* cadena = obtener_resultado(resultado);

			log_info(logger_team, "Recibí un mensaje de tipo CAUGHT_POKEMON y sus datos son: %d %s\n", id_correlativo, cadena);

			free(cadena);

			for (int i = 0; i < list_size(entrenadores); i++) {
				t_entrenador* entrenador = list_get(entrenadores, i);
				if (entrenador->id_caught == id_correlativo) {
					entrenador->resultado_caught = resultado;
					sem_post(&(llega_mensaje_caught[entrenador->indice]));
				}
			}

			confirmar_recepcion(id, id_cola_caught, "CAUGHT_POKEMON");

			break;
		}
		case LOCALIZED_POKEMON:{
			id = recibir_entero(cliente_fd);
			size = recibir_entero(cliente_fd);
			id_correlativo = recibir_entero(cliente_fd);
			uint32_t size_pokemon;
			char* pokemon = recibir_cadena(cliente_fd, &size_pokemon);
			uint32_t cantidad_posiciones = recibir_entero(cliente_fd);
			uint32_t pos_x;
			uint32_t pos_y;

			char* auxiliar = string_new();



			for (int i = 0; i < cantidad_posiciones; i++) {
				pos_x = recibir_entero(cliente_fd);
				string_append_with_format(&auxiliar, "%d ", pos_x);
				pos_y = recibir_entero(cliente_fd);
				string_append_with_format(&auxiliar, "%d ", pos_y);



				if((esta_en_ids_get(id_correlativo)) && (sigue_en_falta_especie(pokemon)) && (list_elem(pokemon, objetivo_global)) && (!ya_recibio_especie(pokemon))){
					if (get_algoritmo_planificacion() == SJFCD) sem_wait(&puede_ser_pusheado);
					queue_push(appeared_pokemons, crear_localized_pokemon(pokemon, pos_x, pos_y));
					sem_post(&sem_appeared_pokemon);

					/*if (sigue_en_falta_especie(pokemon)) { //&& (!ya_recibio_especie())
						if(get_algoritmo_planificacion() == SJFCD) sem_wait(&puede_ser_pusheado);
						queue_push(appeared_pokemons, crear_localized_pokemon(pokemon, pos_x, pos_y));
						sem_post(&sem_appeared_pokemon);
					}*/

				}
			}
			recibir_pokemon(pokemon);



			log_info(logger_team, "Recibí un mensaje de tipo LOCALIZED_POKEMON y sus datos son: %s %d %s\n", pokemon, cantidad_posiciones, auxiliar);
			free(auxiliar);

			confirmar_recepcion(id, id_cola_localized, "LOCALIZED_POKEMON");
			break;
		}
		case SUSCRIPTOR: {
			u_int32_t id_cola = recibir_entero(cliente_fd);
			u_int32_t size_2 = recibir_entero(cliente_fd);
			u_int32_t id_correlativo = recibir_entero(cliente_fd);
			tipo_mensaje tipo = recibir_entero(cliente_fd);
			asignar_id_cola_de_mensajes(id_cola, tipo);

			break;
		}
		case 0:
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
	}
}

void asignar_id_caught(t_entrenador* entrenador, int conexion) {
	tipo_mensaje tipo = recibir_entero(conexion);
	if (tipo == CATCH_POKEMON) {
		entrenador->id_caught = recibir_entero(conexion);
		recibir_entero(conexion);
		recibir_entero(conexion);
		int size;
		char* cadena = recibir_cadena(conexion, &size);
		recibir_entero(conexion);
		recibir_entero(conexion);
		free(cadena);
	}
	else exit(1);
	// Ale dijo que no hay que hacer free de los mensajes
}

void asignar_id_get(int conexion) {
	tipo_mensaje tipo = recibir_entero(conexion);
	if (tipo == GET_POKEMON) {
		uint32_t id = recibir_entero(conexion);
		list_add(ids_gets, (void*) id);
		int size = recibir_entero(conexion); //Buffer size
		recibir_entero(conexion); //id Correlativo
		char* cadena = recibir_cadena(conexion, &size);

		free(cadena);
	}
	else exit(1);
	// Ale dijo que no hay que hacer free de los mensajes
}

void asignar_id_cola_de_mensajes(u_int32_t id_a_asignar, tipo_mensaje tipo) {
	char* auxiliar;
	switch (tipo) {
		case APPEARED_POKEMON:
			id_cola_appeared = id_a_asignar;
			auxiliar = string_itoa(id_a_asignar);
			config_set_value(config, "ID_COLA_APPEARED", auxiliar);
			config_save(config);
			free(auxiliar);
			break;
		case LOCALIZED_POKEMON:
			id_cola_localized = id_a_asignar;
			auxiliar = string_itoa(id_a_asignar);
			config_set_value(config, "ID_COLA_LOCALIZED", auxiliar);
			config_save(config);
			free(auxiliar);
			break;
		case CAUGHT_POKEMON:
			id_cola_caught = id_a_asignar;
			auxiliar = string_itoa(id_a_asignar);
			config_set_value(config, "ID_COLA_CAUGHT", auxiliar);
			config_save(config);
			free(auxiliar);
			break;
		default:
			break;
	}
}

/*
 * @NAME: recibir_entero
 * @DESC: Dado un socket_cliente, recibe un entero desde ese socket.
 */
u_int32_t recibir_entero(int socket_cliente) {
	int entero;
	recv(socket_cliente, &entero, sizeof(int), MSG_WAITALL);

	return entero;
}

// FUNCIONES CLIENTE

/*
 * @NAME: serializar_paquete
 * @DESC: Dado un paquete y una cantidad de bytes,
 * 		  serializa ese paquete.
 */
void* serializar_paquete(t_paquete* paquete, u_int32_t *bytes) {
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
int crear_conexion(char *ip, char* puerto) {
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	u_int32_t socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) < 0) {
		log_error(logger_team, "No se pudo establecer la conexion con el Broker\n");
		freeaddrinfo(server_info);
		return -1;
	}
	else log_info(logger_team, "Se pudo establecer la conexion con el Broker\n");

	freeaddrinfo(server_info);

	return socket_cliente;
}

int crear_y_reintentar_conexion(char *ip, char* puerto) {
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	u_int32_t socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	while (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) < 0) {
		log_error(logger_team, "No se pudo establecer la conexion con el Broker\n");
		log_warning(logger_team, "Se inicia el proceso de reintento de comunicacion con el Broker\n");
		sleep(config_team->tiempo_reconexion);
		if (inicio_deadlock){
			freeaddrinfo(server_info);
			return -1;
		}
	}

	log_info(logger_team, "Se pudo establecer la conexion con el Broker\n");

	freeaddrinfo(server_info);

	return socket_cliente;
}

/*
 * @NAME: enviar_mensaje
 * @DESC: Dado un arreglo de argumentos en formato string,
 * 		  y un socket_cliente, manda un mensaje a ese socket.
 */
void enviar_mensaje(char* argv[], u_int32_t socket_cliente) {
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

	sem_wait(&(mutex_ciclos_cpu_totales));
	ciclos_cpu_totales += ENVIAR_MENSAJE;
	sem_post(&(mutex_ciclos_cpu_totales));

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
void agregar_string(int* offset, char* string, void** stream) {
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
void agregar_entero(int* offset, char* string, void** stream) {
	u_int32_t entero = atoi(string);
	memcpy((*stream) + (*offset), &entero, sizeof(u_int32_t));
	(*offset) += sizeof(u_int32_t);
}

/*
 * @NAME: generar_stream
 * @DESC: Dados un arreglo de argumentos en formato de string y un paquete,
 * 		  genera el stream correspondiente a ese paquete.
 */
void* generar_stream(char** argumentos, t_paquete* paquete) {
	int offset = 0;
	void* stream = malloc(paquete->buffer->size);

	switch (paquete->codigo_operacion) {
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
		case CONFIRMAR: {
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
u_int32_t obtener_size(char* argumentos[], tipo_mensaje tipo) {
	u_int32_t size = 0;
	switch (tipo) {
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
void liberar_conexion(u_int32_t socket_cliente) {
	close(socket_cliente);
}
