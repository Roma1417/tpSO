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
tipo_mensaje obtener_tipo_mensaje(char* tipo) {
	tipo_mensaje tipo_mensaje;
	if (strcasecmp(tipo, "NEW_POKEMON") == 0) {
		tipo_mensaje = NEW_POKEMON;
	} else if (strcasecmp(tipo, "APPEARED_POKEMON") == 0) {
		tipo_mensaje = APPEARED_POKEMON;
	} else if (strcasecmp(tipo, "CATCH_POKEMON") == 0) {
		tipo_mensaje = CATCH_POKEMON;
	} else if (strcasecmp(tipo, "CAUGHT_POKEMON") == 0) {
		tipo_mensaje = CAUGHT_POKEMON;
	} else if (strcasecmp(tipo, "GET_POKEMON") == 0) {
		tipo_mensaje = GET_POKEMON;
	} else if (strcasecmp(tipo, "LOCALIZED_POKEMON") == 0) {
		tipo_mensaje = LOCALIZED_POKEMON;
	} else if (strcasecmp(tipo, "SUSCRIPTOR") == 0) {
		tipo_mensaje = SUSCRIPTOR;
	} else if (strcasecmp(tipo, "CONFIRMAR") == 0) {
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
	if (tipo == NEW_POKEMON)
		return "NEW_POKEMON";
	if (tipo == APPEARED_POKEMON)
		return "APPEARED_POKEMON";
	if (tipo == CATCH_POKEMON)
		return "CATCH_POKEMON";
	if (tipo == CAUGHT_POKEMON)
		return "CAUGHT_POKEMON";
	if (tipo == GET_POKEMON)
		return "GET_POKEMON";
	if (tipo == LOCALIZED_POKEMON)
		return "LOCALIZED";
	if (tipo == SUSCRIPTOR)
		return "SUSCRIPTOR";
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
	cadena[(*size)] = '\0';
	return cadena;
}

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
		if ((socket_servidor = socket(p->ai_family, p->ai_socktype,
						p->ai_protocol)) == -1)
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
void esperar_cliente(int socket_servidor) {

	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente,
					&tam_direccion);

	pthread_create(&thread, NULL, (void*) serve_client, &socket_cliente);
	pthread_join(thread, NULL);

}

/*
 * @NAME: serve_client
 * @DESC: Funcion auxilar de iniciar_servidor.
 */
void recibir_mensaje(int* socket) {

	int cod_op;

	if (recv(*socket, &cod_op, sizeof(int), MSG_WAITALL) == -1)
		cod_op = -1;
	process_request(cod_op, *socket);
}

void confirmar_recepcion(u_int32_t id_mensaje, u_int32_t id_proceso, char* mensaje) {
	int cliente_fd = crear_conexion(config_gamecard->ip_broker, config_gamecard->puerto_broker);
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
	liberar_conexion(cliente_fd);
}

bool list_elem(char* elemento, t_list* lista) {
	bool encontrado = false;
	for (int i = 0; i < list_size(lista) && !encontrado; i++) {
		char* pokemon = list_get(lista, i);
		encontrado = string_equals_ignore_case(pokemon, elemento);
	}
	return encontrado;
}

void cargar_datos_new_pokemon(t_new_pokemon* new_pokemon) {
	verificar_existencia_de_archivo(new_pokemon->pokemon);
	char* file_pokemon_path = generar_pokemon_metadata_bin_path(
					new_pokemon->pokemon);
	FILE* file_pokemon = fopen(file_pokemon_path, "r+");
	free(file_pokemon_path);
	verificar_estado_de_apertura_de_archivo_pokemon(file_pokemon);
	actualizar_posiciones(file_pokemon, new_pokemon);
	cerrar_file(file_pokemon);
	fclose(file_pokemon);
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
	uint32_t id_correlativo;
	u_int32_t posicion_x;
	u_int32_t posicion_y;
	printf("Cod_op: %d\n", cod_op);
	switch (cod_op) {
		case NEW_POKEMON:
			printf("Recibi un mensaje NEW_POKEMON\n");
			t_new_pokemon* new_pokemon = malloc(sizeof(t_new_pokemon));
			id = recibir_entero(cliente_fd);
			size = recibir_entero(cliente_fd);
			id_correlativo = recibir_entero(cliente_fd);
			new_pokemon->pokemon = recibir_cadena(cliente_fd, &size_pokemon);
			printf("Pokemon: %s\n", new_pokemon->pokemon);
			new_pokemon->pos_x = recibir_entero(cliente_fd);
			new_pokemon->pos_y = recibir_entero(cliente_fd);
			new_pokemon->cantidad = recibir_entero(cliente_fd);

			log_info(logger_gamecard,
							"Recibí un mensaje de tipo NEW_POKEMON y sus datos son: %s %d %d %d",
							new_pokemon->pokemon, new_pokemon->pos_x, new_pokemon->pos_y,
							new_pokemon->cantidad);

			confirmar_recepcion(id, id_cola_new, "NEW_POKEMON");

			cargar_datos_new_pokemon(new_pokemon);

			sleep(config_gamecard->tiempo_de_reintento_operacion);

			enviar_appeared_pokemon(id, new_pokemon);

			break;
		case CATCH_POKEMON:
			;
			//printf("Recibi un mensaje CATCH_POKEMON\n");
			t_catch_pokemon* catch_pokemon = malloc(sizeof(t_catch_pokemon));
			id = recibir_entero(cliente_fd);
			printf("id: %d\n", id);
			size = recibir_entero(cliente_fd);
			printf("size: %d\n", size);
			id_correlativo = recibir_entero(cliente_fd);
			printf("id_correlativo: %d\n", id_correlativo);
			catch_pokemon->pokemon = recibir_cadena(cliente_fd, &size_pokemon);
			printf("catch_pokemon->pokemon : %s\n", catch_pokemon->pokemon);
			catch_pokemon->pos_x = recibir_entero(cliente_fd);
			printf("catch_pokemon->pos_x : %d\n", catch_pokemon->pos_x);
			catch_pokemon->pos_y = recibir_entero(cliente_fd);
			printf("catch_pokemon->pos_y : %d\n", catch_pokemon->pos_y);

			log_info(logger_gamecard, "Recibí un mensaje de tipo CATCH_POKEMON y sus datos son: %s %d %d", catch_pokemon->pokemon, catch_pokemon->pos_x, catch_pokemon->pos_y);

			confirmar_recepcion(id, id_cola_catch, "CATCH_POKEMON");

			bool resultado_captura = generar_resultado_captura(catch_pokemon);

			sleep(config_gamecard->tiempo_de_reintento_operacion);
			enviar_caught_pokemon(id, resultado_captura);

			break;
		case GET_POKEMON:
			//printf("Recibi un mensaje GET_POKEMON\n");
			id = recibir_entero(cliente_fd);
			printf("id: %d\n", id);
			size = recibir_entero(cliente_fd);
			printf("size: %d\n", size);
			id_correlativo = recibir_entero(cliente_fd);
			printf("id_correlativo: %d\n", id_correlativo);
			pokemon = recibir_cadena(cliente_fd, &size_pokemon);
			printf("Pokemon: %s\n", pokemon);

			log_info(logger_gamecard,
							"Recibí un mensaje de tipo GET_POKEMON y sus datos son: %s",
							pokemon);

			confirmar_recepcion(id, id_cola_get, "GET_POKEMON");

			t_list* posiciones = obtener_posiciones_del_pokemon(pokemon);

			for (int i = 0; i < list_size(posiciones); i++)
				printf("Posicion %d: %s\n", i + 1, list_get(posiciones, i));

			sleep(config_gamecard->tiempo_de_reintento_operacion);

			enviar_mensaje_localized(pokemon, posiciones, id);

			break;

		case SUSCRIPTOR: {
			u_int32_t id_cola = recibir_entero(cliente_fd);
			u_int32_t size_2 = recibir_entero(cliente_fd);
			id_correlativo = recibir_entero(cliente_fd);
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

void asignar_id_cola_de_mensajes(u_int32_t id_a_asignar, tipo_mensaje tipo) {
	switch (tipo) {
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

void enviar_appeared_pokemon(u_int32_t id_mensaje, t_new_pokemon* new_pokemon) {
	uint32_t conexion = crear_conexion(config_gamecard->ip_broker, config_gamecard->puerto_broker);
	char** mensaje_appeared_pokemon = malloc((sizeof(char*)) * 6);
	mensaje_appeared_pokemon[0] = string_new();
	string_append(&(mensaje_appeared_pokemon[0]), "BROKER");
	mensaje_appeared_pokemon[1] = string_new();
	string_append(&(mensaje_appeared_pokemon[1]), "APPEARED_POKEMON");
	mensaje_appeared_pokemon[2] = string_itoa(id_mensaje);
	mensaje_appeared_pokemon[3] = new_pokemon->pokemon;
	mensaje_appeared_pokemon[4] = string_itoa(new_pokemon->pos_x);
	mensaje_appeared_pokemon[5] = string_itoa(new_pokemon->pos_y);
	enviar_mensaje(mensaje_appeared_pokemon, conexion);
	for (int i = 0; i < 6; i++)
		free(mensaje_appeared_pokemon[i]);
	free(mensaje_appeared_pokemon);
	liberar_conexion(conexion);
}

bool existe_directorio_pokemon(char* pokemon) {
	char* path_pokemon_file = generar_pokemon_file_path(pokemon);
	DIR* directorio_pokemon = opendir(path_pokemon_file);
	bool existe_directorio = directorio_pokemon != NULL;
	free(path_pokemon_file);
	closedir(directorio_pokemon);
	return existe_directorio;
}

void actualizar_size_catch_pokemon(t_list* posiciones, FILE** file_pokemon, t_catch_pokemon* catch_pokemon) {
	fseek(*file_pokemon, 0, SEEK_SET);
	char* contenido = string_new();
	char* auxiliar = guardar_hasta('Z', file_pokemon);
	string_append(&contenido, auxiliar);
	string_append(&contenido, "Z");
	free(auxiliar);
	auxiliar = guardar_hasta('=', file_pokemon);
	string_append(&contenido, auxiliar);
	free(auxiliar);
	string_append(&contenido, "=");
	uint32_t size_total = 0;
	for (int i = 0; i < list_size(posiciones); i++) {
		size_total += string_length(list_get(posiciones, i));
	}
	string_append(&contenido, string_itoa(size_total));
	printf("Llegué hasta acá\n");
	avanzar_hasta('\n', file_pokemon);
	string_append(&contenido, "\n");
	auxiliar = guardar_hasta_EOF(file_pokemon);
	string_append(&contenido, auxiliar);
	free(auxiliar);
	fclose(*file_pokemon);
	char* file_pokemon_path = generar_pokemon_metadata_bin_path(catch_pokemon->pokemon);
	*file_pokemon = fopen(file_pokemon_path, "wb");
	printf("Contenido: %s\n", contenido);
	fputs(contenido, *file_pokemon);
	printf("Y hasta acá\n");
	//cerrar_file(file_pokemon);
	//fclose(file_pokemon);
}

t_list* reiniciar_bloques_file(t_list* bloques_file, FILE** bloque_file,
				t_list* bloques) {
	cerrar_bloques_file(bloques_file, *bloque_file);
	list_destroy(bloques_file);
	bloques_file = list_create();
	char* bloque_path;
	for (int i = 0; i < list_size(bloques); i++) {
		bloque_path = obtener_bloque_path(list_get(bloques, i));
		printf("Bloque_path: %s\n", bloque_path);
		*bloque_file = fopen(bloque_path, "wb");
		list_add(bloques_file, *bloque_file);
		free(bloque_path);
	}
	return bloques_file;
}

void quitar_ultimo_bloque(FILE** file_pokemon, t_list* bloques, char* pokemon) {
	fseek(*file_pokemon, 0, SEEK_SET);
	printf("WardA\n");
	char* anterior = guardar_hasta('B', file_pokemon);
	printf("WardB\n");
	string_append_with_format(&anterior, "%c", 'B');
	string_append(&anterior, guardar_hasta('=', file_pokemon));
	printf("WardC\n");
	string_append_with_format(&anterior, "%c", '=');
	printf("Anterior: %s\n", anterior);
	avanzar_hasta('\n', file_pokemon);
	char* posterior = guardar_hasta_EOF(file_pokemon);
	printf("Posterior: %s\n", posterior);
	fseek(*file_pokemon, -string_length(posterior), SEEK_CUR);
	retroceder_hasta('=', file_pokemon);
	char* array_bloques = string_new();
	string_append_with_format(&array_bloques, "%c", '[');
	for (int i = 0; i < (list_size(bloques) - 1); i++) {
		string_append(&array_bloques, list_get(bloques, i));
		if (i != (list_size(bloques) - 2))
			string_append_with_format(&array_bloques, "%c", ',');
	}
	string_append(&array_bloques, "]\n");
	printf("Blocks: %s\n", array_bloques);
	fclose(*file_pokemon);
	char* file_pokemon_path = generar_pokemon_metadata_bin_path(pokemon);
	*file_pokemon = fopen(file_pokemon_path, "wb");
	fseek(*file_pokemon, 0, SEEK_SET);
	fputs(anterior, *file_pokemon);
	fputs(array_bloques, *file_pokemon);
	fputs(posterior, *file_pokemon);
	fseek(*file_pokemon, 0, SEEK_SET);

	free(anterior);
	free(array_bloques);
	free(posterior);
}

t_list* capturar_pokemon(FILE* file_pokemon, t_list* posiciones,
				char* posicion_actual, t_list* bloques_file, FILE** bloque_file,
				t_list* bloques, t_catch_pokemon* catch_pokemon) {
	//Esto se va a delegar
	char* ultimo_bloque = obtener_ultimo_bloque(file_pokemon);
	int indice_del_encontrado = 0;
	obtener_indice_del_encontrado(&indice_del_encontrado, posiciones,
					posicion_actual);
	char* posicion = list_get(posiciones, indice_del_encontrado);
	uint32_t nueva_cantidad = obtener_cantidad(posicion) - 1;
	if (nueva_cantidad == 0) {
		list_remove(posiciones, indice_del_encontrado);
		bloques_file = reiniciar_bloques_file(bloques_file, bloque_file, bloques);
	} else {
		posicion = cargar_nueva_posicion(nueva_cantidad, posicion);
		list_replace(posiciones, indice_del_encontrado, posicion);
		bloques_file = reiniciar_bloques_file(bloques_file, bloque_file, bloques);
		posicionar_en_inicio(bloques_file, *bloque_file);
	}

	actualizar_posiciones_ya_cargadas(posiciones, *bloque_file, bloques_file,
					file_pokemon, ultimo_bloque);
	printf("Llegué aca\n");
	actualizar_size_catch_pokemon(posiciones, &file_pokemon, catch_pokemon);

	cerrar_file(file_pokemon);
	fclose(file_pokemon);
	char* file_pokemon_path = generar_pokemon_metadata_bin_path(catch_pokemon->pokemon);
	file_pokemon = fopen(file_pokemon_path, "r");

	float contador = 0;
	char* posicion_auxiliar;
	for (int i = 0; i < list_size(posiciones); i++) {
		posicion_auxiliar = list_get(posiciones, i);
		for (int j = 0; j < string_length(posicion_auxiliar); j++) {
			contador++;
		}
	}
	printf("Contador: %d\n", contador);
	float division = contador / metadata_general->block_size;
	printf("NUMERO: %2.f\n", division);
	if (division <= (list_size(bloques_file) - 1)) {
		quitar_ultimo_bloque(&file_pokemon, bloques, catch_pokemon->pokemon);
		uint32_t numero_de_bloque = atoi(list_get(bloques, list_size(bloques) - 1)) - 1;
		bitarray_clean_bit(bitmap, numero_de_bloque);
		actualizar_bit_map();
		list_remove(bloques, list_size(bloques) - 1);
		fclose(list_get(bloques_file, list_size(bloques_file) - 1));
		list_remove(bloques_file, list_size(bloques_file) - 1);

	}

	cerrar_file(file_pokemon);
	fclose(file_pokemon);

	return bloques_file;
}

bool generar_resultado_captura(t_catch_pokemon* catch_pokemon) {
	printf("Llegué a la función\n");
	if (!existe_directorio_pokemon(catch_pokemon->pokemon))
		return false;
	printf("Existe el archivo\n");
	char* file_pokemon_path = generar_pokemon_metadata_bin_path(catch_pokemon->pokemon);
	FILE* file_pokemon = fopen(file_pokemon_path, "r+");
	verificar_estado_de_apertura_de_archivo_pokemon(file_pokemon);
	printf("Pude abrir el archivo\n");

	t_list* bloques = obtener_bloques_del_pokemon(file_pokemon);
	t_list* bloques_file = obtener_bloques_actuales(file_pokemon, bloques);
	t_list* posiciones = obtener_posiciones_actuales(file_pokemon, bloques_file, bloques);
	FILE* bloque_file = NULL;
	char* posicion_actual = string_new();
	string_append_with_format(&posicion_actual, "%s-%s=", string_itoa(catch_pokemon->pos_x), string_itoa(catch_pokemon->pos_y));

	if (!posicion_ya_cargada(posicion_actual, posiciones))
		return false;
	printf("Existe la posicion buscada\n");

	bloques_file = capturar_pokemon(file_pokemon, posiciones, posicion_actual, bloques_file, &bloque_file, bloques, catch_pokemon);
	cerrar_bloques_file(bloques_file, bloque_file);

	return true;
}

void enviar_caught_pokemon(uint32_t id_mensaje, bool resultado_catch) {
	int conexion = crear_conexion(config_gamecard->ip_broker, config_gamecard->puerto_broker);
	char** mensaje_appeared_pokemon = malloc((sizeof(char*)) * 4);
	mensaje_appeared_pokemon[0] = string_new();
	string_append(&(mensaje_appeared_pokemon[0]), "BROKER");
	mensaje_appeared_pokemon[1] = string_new();
	string_append(&(mensaje_appeared_pokemon[1]), "CAUGHT_POKEMON");
	mensaje_appeared_pokemon[2] = string_itoa(id_mensaje);
	mensaje_appeared_pokemon[3] = string_itoa(resultado_catch);
	enviar_mensaje(mensaje_appeared_pokemon, conexion);
	liberar_conexion(conexion);
}

char* quitar_cantidad(char* posicion) {
	char** posicion_en_partes = string_split(posicion, "=");
	posicion = posicion_en_partes[0];
	printf("Posicion sin cantidad: %s\n", posicion);
	return posicion;
}

t_list* obtener_posiciones_del_pokemon(char* pokemon) {
	if (!existe_directorio_pokemon(pokemon))
		return list_create();
	char* file_pokemon_path = generar_pokemon_metadata_bin_path(pokemon);
	FILE* file_pokemon = fopen(file_pokemon_path, "r+");
	verificar_estado_de_apertura_de_archivo_pokemon(file_pokemon);

	t_list* bloques = obtener_bloques_del_pokemon(file_pokemon);
	char* auxiliar = list_get(bloques,0);
	printf("Auxiliar: %s\n", auxiliar);
	if(string_equals_ignore_case(auxiliar,"")) {
		fclose(file_pokemon);
		return list_create();
	}
	t_list* bloques_file = obtener_bloques_actuales(file_pokemon, bloques);
	t_list* posiciones = obtener_posiciones_actuales(file_pokemon, bloques_file, bloques);
	FILE* bloque_file = NULL;
	t_list* posiciones_sin_cantidad = list_map(posiciones, (void*) quitar_cantidad);
	cerrar_file(file_pokemon);
	fclose(file_pokemon);
	return posiciones_sin_cantidad;
}

void enviar_mensaje_localized(char* pokemon, t_list* posiciones, uint32_t id) {
	uint32_t cantidad_posiciones = list_size(posiciones);
	printf("Cantidad de posiciones: %d\n", cantidad_posiciones);
	char* pos_x;
	char* pos_y;
	char* posicion_actual;

	int conexion = crear_conexion(config_gamecard->ip_broker, config_gamecard->puerto_broker);
	char** mensaje_localized_pokemon = malloc((sizeof(char*)) * (5 + cantidad_posiciones * 2));
	mensaje_localized_pokemon[0] = string_new();
	string_append(&(mensaje_localized_pokemon[0]), "BROKER");
	mensaje_localized_pokemon[1] = string_new();
	string_append(&(mensaje_localized_pokemon[1]), "LOCALIZED_POKEMON");
	mensaje_localized_pokemon[2] = string_itoa(id);
	mensaje_localized_pokemon[3] = pokemon;
	mensaje_localized_pokemon[4] = string_itoa(cantidad_posiciones);
	int k = 5;
	for (int i = 0; i < cantidad_posiciones; i++) {
		posicion_actual = list_get(posiciones, i);
		printf("Posicion_actual: %s\n", posicion_actual);
		mensaje_localized_pokemon[k] = string_new();
		mensaje_localized_pokemon[k + 1] = string_new();

		int j = 0;
		for (; posicion_actual[j] != '-'; j++) {
			printf("Caracter_cargado: %c\n", posicion_actual[j]);
			string_append_with_format(&(mensaje_localized_pokemon[k]), "%c", posicion_actual[j]);
		}
		j++;
		for (; j < string_length(posicion_actual); j++) {
			string_append_with_format(&(mensaje_localized_pokemon[k + 1]), "%c", posicion_actual[j]);
		}
		k += 2;
	}

	for (int i = 0; i < (5 + cantidad_posiciones * 2); i++)
		printf("Mensaje[%d]: %s\n", i, mensaje_localized_pokemon[i]);

	enviar_mensaje(mensaje_localized_pokemon, conexion);
	printf("Envié localized_pokemon\n");
	liberar_conexion(conexion);
}

void serve_client(int* cliente_fd) {
	int cod_op;

	if (recv(*cliente_fd, &cod_op, sizeof(int), MSG_WAITALL) == -1)
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
			t_new_pokemon* new_pokemon = malloc(sizeof(t_new_pokemon));
			size = recibir_entero(*cliente_fd);
			new_pokemon->pokemon = recibir_cadena(*cliente_fd, &size_pokemon);
			printf("Pokemon recibido: %s\n", new_pokemon->pokemon);
			new_pokemon->pos_x = recibir_entero(*cliente_fd);
			new_pokemon->pos_y = recibir_entero(*cliente_fd);
			new_pokemon->cantidad = recibir_entero(*cliente_fd);
			id_mensaje = recibir_entero(*cliente_fd);

			cargar_datos_new_pokemon(new_pokemon);

			sleep(config_gamecard->tiempo_de_reintento_operacion);

			enviar_appeared_pokemon(id_mensaje, new_pokemon);

			//free(new_pokemon->pokemon);
			free(new_pokemon);

			break;
		case CATCH_POKEMON:
			printf("Recibi un mensaje CATCH_POKEMON\n");
			t_catch_pokemon* catch_pokemon = malloc(sizeof(t_catch_pokemon));
			size = recibir_entero(*cliente_fd);
			catch_pokemon->pokemon = recibir_cadena(*cliente_fd, &size_pokemon);
			catch_pokemon->pos_x = recibir_entero(*cliente_fd);
			catch_pokemon->pos_y = recibir_entero(*cliente_fd);
			id_mensaje = recibir_entero(*cliente_fd);

			bool resultado_captura = generar_resultado_captura(catch_pokemon);
			sleep(config_gamecard->tiempo_de_reintento_operacion);

			enviar_caught_pokemon(id_mensaje, resultado_captura);

			break;
		case GET_POKEMON:
			printf("Recibi un mensaje GET_POKEMON\n");
			size = recibir_entero(*cliente_fd);
			printf("size: %d\n", size);
			pokemon = recibir_cadena(*cliente_fd, &size_pokemon);
			printf("size_Pokemon: %d\n", size_pokemon);
			printf("Pokemon: %s\n", pokemon);
			id_mensaje = recibir_entero(*cliente_fd);

			t_list* posiciones = obtener_posiciones_del_pokemon(pokemon);

			for (int i = 0; i < list_size(posiciones); i++)
				printf("Posicion %d: %s\n", i + 1, list_get(posiciones, i));

			sleep(config_gamecard->tiempo_de_reintento_operacion);

			enviar_mensaje_localized(pokemon, posiciones, id_mensaje);

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
void* serializar_paquete(t_paquete* paquete, u_int32_t *bytes) {
	u_int32_t size_serializado = sizeof(paquete->codigo_operacion)
					+ sizeof(paquete->buffer->size) + paquete->buffer->size;
	void* buffer = malloc(size_serializado);
	u_int32_t bytes_escritos = 0;

	memcpy(buffer + bytes_escritos, &(paquete->codigo_operacion),
					sizeof(paquete->codigo_operacion));
	bytes_escritos += sizeof(paquete->codigo_operacion);

	memcpy(buffer + bytes_escritos, &(paquete->buffer->size),
					sizeof(paquete->buffer->size));
	bytes_escritos += sizeof(paquete->buffer->size);

	memcpy(buffer + bytes_escritos, paquete->buffer->stream,
					paquete->buffer->size);
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

	u_int32_t socket_cliente = socket(server_info->ai_family,
					server_info->ai_socktype, server_info->ai_protocol);

	if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) < 0) {
		freeaddrinfo(server_info);
		return -1;
	}

	/*if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen)
	 == -1)
	 printf("error");*/

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
	printf("Cod_op: %d\n", tipo);
	paquete->buffer = malloc(sizeof(t_buffer));
	u_int32_t size = obtener_size(argv, tipo);
	paquete->buffer->size = size;
	void* stream = generar_stream(argv, paquete);
	paquete->buffer->stream = stream;
	printf("SIZE: %d\n", size);
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
		case APPEARED_POKEMON:
			agregar_entero(&offset, argumentos[2], &stream);
			agregar_string(&offset, argumentos[3], &stream);
			agregar_entero(&offset, argumentos[4], &stream);
			agregar_entero(&offset, argumentos[5], &stream);
			break;
		case CAUGHT_POKEMON:
			agregar_entero(&offset, argumentos[2], &stream);
			agregar_entero(&offset, argumentos[3], &stream);
			break;
		case LOCALIZED_POKEMON:
			agregar_entero(&offset, argumentos[2], &stream);
			agregar_string(&offset, argumentos[3], &stream);
			agregar_entero(&offset, argumentos[4], &stream);
			int indice = 5;
			for (int i = 0; i < atoi(argumentos[4]); i++) {
				agregar_entero(&offset, argumentos[indice], &stream);
				indice++;
				agregar_entero(&offset, argumentos[indice], &stream);
				indice++;
			}
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
		case APPEARED_POKEMON:
			size = sizeof(u_int32_t) * 4 + strlen(argumentos[3]);
			break;
		case CAUGHT_POKEMON:
			size = sizeof(u_int32_t) * 2;
			break;
		case LOCALIZED_POKEMON:
			size = sizeof(u_int32_t) * (3 + 2 * atoi(argumentos[4])) + strlen(argumentos[3]);
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

