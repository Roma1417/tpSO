/*
 * gamecard.c
 *
 *  Created on: 28 may. 2020
 *      Author: utnso
 */
#include "gamecard.h"

t_config* leer_config() {
	t_config* config = config_create("./gamecard.config");
	return config;
}

t_log* iniciar_logger() {

	t_log* logger = log_create("gamecard.log", "gamecard", true, LOG_LEVEL_INFO);
	if (logger == NULL) {
		exit(1);
	}
	return logger;

}

void* mantener_servidor() {

	iniciar_servidor();

	return EXIT_SUCCESS;
}

char* id_cola_mensajes(char* msg) {
	char* id_cola;
	if (string_equals_ignore_case(msg, "GET_POKEMON"))
		id_cola = string_itoa(id_cola_get);
	else if (string_equals_ignore_case(msg, "NEW_POKEMON"))
		id_cola = string_itoa(id_cola_new);
	else if (string_equals_ignore_case(msg, "CATCH_POKEMON"))
		id_cola = string_itoa(id_cola_catch);

	return id_cola;
}

void* suscribirse(void* cola) {

	char* msg = (char *) cola;
	int conexion = crear_conexion(config_gamecard->ip_broker, config_gamecard->puerto_broker);

	if (conexion < 0) {
		return EXIT_SUCCESS;
	}

	char** mensaje = malloc(sizeof(char*) * 4);

	mensaje[0] = string_new();
	string_append(&(mensaje[0]), "BROKER");

	mensaje[1] = string_new();
	string_append(&(mensaje[1]), "SUSCRIPTOR");

	mensaje[2] = string_new();
	string_append(&(mensaje[2]), msg);

	mensaje[3] = id_cola_mensajes(msg);

	// Y si establece conexion, no se envian mensajes bien

	enviar_mensaje(mensaje, conexion);

	free(msg);

	//Falta esperar la respuesta

	//pthread_t thread_suscriptor;

	//serve_client(&conexion);

	/*while (1){
	 pthread_create(&thread_suscriptor,NULL,(void*)serve_client,&conexion);
	 pthread_join(thread_suscriptor, NULL);
	 }*/
	while (1) {
		pthread_create(&thread, NULL, (void*) recibir_mensaje, &conexion);
		pthread_join(thread, NULL);
	}

	// Problema para ale -> no liberar conexion
	liberar_conexion(conexion);

	return EXIT_SUCCESS;
}

/*
 * @NAME: suscribirse_a_colas
 * @DESC: Suscribe al proceso Team a las colas de mensajes del
 * 		  broker.
 */
void suscribirse_a_colas() {

	// REVISAR
	char* mensaje = string_new();
	string_append(&mensaje, "NEW_POKEMON");
	pthread_create(&hilo_new, NULL, suscribirse, (void*) mensaje);

	char* mensaje2 = string_new();
	string_append(&mensaje2, "GET_POKEMON");
	pthread_create(&hilo_get, NULL, suscribirse, (void*) mensaje2);

	char* mensaje3 = string_new();
	string_append(&mensaje3, "CATCH_POKEMON");
	pthread_create(&hilo_catch, NULL, suscribirse, (void*) mensaje3);

}

void* iniciar_hilo_verificador_de_conexion() {
	int conexion;
	while (1) {
		int conexion = crear_conexion(config_gamecard->ip_broker, config_gamecard->puerto_broker);
		if (conexion < 0) {
			log_info(logger_gamecard, "No se pudo establecer la conexion con el Broker\n");
			log_info(logger_gamecard, "Se inicia el proceso de reintento de comunicacion con el Broker\n");
			pthread_cancel(hilo_catch);
			pthread_cancel(hilo_get);
			pthread_cancel(hilo_new);
			sleep(config_gamecard->tiempo_de_reintento_conexion);
			suscribirse_a_colas();
		} else {
			liberar_conexion(conexion);
			sleep(config_gamecard->tiempo_de_reintento_conexion);
		}

	}

	return EXIT_SUCCESS;
}

/**
 * @NAME: construir_config_gamecard
 * @DESC: Crea y devuelve un puntero a una estructura t_config_gamecard,
 * 		 donde sus valores internos corresponden a los guardados en
 * 		 el archivo del gamecard de tipo config.
 */
t_config_gamecard* construir_config_gamecard(t_config* config) {

	t_config_gamecard* config_gamecard_local = malloc(sizeof(t_config_gamecard));

	config_gamecard_local->puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	config_gamecard_local->ip_broker = config_get_string_value(config, "IP_BROKER");
	config_gamecard_local->punto_montaje_tallgrass = config_get_string_value(config, "PUNTO_MONTAJE_TALLGRASS");

	config_gamecard_local->tiempo_de_reintento_operacion = config_get_int_value(config, "TIEMPO_DE_REINTENTO_OPERACION");
	config_gamecard_local->tiempo_de_reintento_conexion = config_get_int_value(config, "TIEMPO_DE_REINTENTO_CONEXION");

	return config_gamecard_local;
}

uint32_t metadata_get_int(FILE* metadata_general_file) {
	char* value = string_new();
	char caracter = fgetc(metadata_general_file);
	while (caracter != '=') {
		caracter = fgetc(metadata_general_file);
	}
	caracter = fgetc(metadata_general_file);
	while ((caracter != '\n') && (caracter != EOF)) {
		string_append_with_format(&value, "%c", caracter);
		caracter = fgetc(metadata_general_file);
	}
	uint32_t valor = atoi(value);
	free(value);
	return valor;
}

char* metadata_get_string(FILE* metadata_general_file) {
	char* value = string_new();
	char caracter = fgetc(metadata_general_file);
	while (caracter != '=') {
		caracter = fgetc(metadata_general_file);
	}
	caracter = fgetc(metadata_general_file);
	while ((caracter != '\n') && (caracter != EOF)) {
		string_append_with_format(&value, "%c", caracter);
		caracter = fgetc(metadata_general_file);
	}
	return value;
}

t_metadata_general* construir_metadata_general() {
	t_metadata_general* metadata_general = malloc(sizeof(t_metadata_general));
	FILE* metadata_general_file = fopen(archivo_metadata_general_path, "r");
	fseek(metadata_general_file, 0, SEEK_SET);
	metadata_general->block_size = metadata_get_int(metadata_general_file);
	metadata_general->blocks = metadata_get_int(metadata_general_file);
	metadata_general->magic_number = metadata_get_string(metadata_general_file);
	fclose(metadata_general_file);
	return metadata_general;
}

void destruir_metadata_general(t_metadata_general* metadata_general) {
	free(metadata_general->magic_number);
	free(metadata_general);
}

void actualizar_bit_map() {

	FILE* bitmap_file = fopen(archivo_bitmap_path, "w");
	fwrite(bitmap->bitarray, metadata_general->blocks / 8, 1, bitmap_file);
	fclose(bitmap_file);

	/*FILE* bitmap_file = fopen(archivo_bitmap_path, "wb+");
	 fwrite(bitmap, metadata_general->blocks / 8, 1, bitmap_file);
	 fclose(bitmap_file);
	 printf("Bit 0:%d --------------\n", bitarray_test_bit(bitmap, 0));
	 printf("YA ACTUALICE EL BITMAP--------------\n");*/
}

void finalizar_gamecard() {
	//free(config_gamecard->ip_broker);
	//free(config_gamecard->puerto_broker);
	//free(config_gamecard->punto_montaje_tallgrass);
	//free(config_gamecard);
	destruir_metadata_general(metadata_general);
	free(archivo_metadata_general_path);
	free(archivo_bitmap_path);
	bitarray_destroy(bitmap);
	pthread_cancel(hilo_catch);
	pthread_cancel(hilo_get);
	pthread_cancel(hilo_new);
	pthread_cancel(hilo_servidor);
	pthread_cancel(thread);
	log_destroy(logger_gamecard);
	config_destroy(config);

	exit(1);
}

sem_t* inicializar_vector_de_semaforos(u_int32_t longitud) {

	sem_t* vector = malloc(sizeof(sem_t) * longitud);
	for (int i = 0; i < longitud; i++) {
		sem_init(&(vector[i]), 0, 0);
	}
	return vector;
}

int main() {
	signal(SIGINT, finalizar_gamecard);
	signal(SIGTERM, finalizar_gamecard);

	config = leer_config();
	config_gamecard = construir_config_gamecard(config);
	verificar_existencia_de_carpeta("/Blocks");
	verificar_existencia_de_carpeta("/Files");
	logger_gamecard = iniciar_logger();
	id_cola_get = 0;
	id_cola_new = 0;
	id_cola_catch = 0;
	archivo_metadata_general_path = generar_nombre("/Metadata/Metadata.bin");
	metadata_general = construir_metadata_general();
	archivo_bitmap_path = generar_nombre("/Metadata/Bitmap.bin");
	FILE* bitmap_file = fopen(archivo_bitmap_path, "r");

	if (bitmap_file == NULL) {

		u_int32_t size = metadata_general->blocks / 8;
		bitmap = bitarray_create_with_mode(string_repeat('\0', size), size, MSB_FIRST);
		actualizar_bit_map();

		/*char* bitarray = malloc((metadata_general->blocks) / 8);
		 bitmap = bitarray_create_with_mode(bitarray, (metadata_general->blocks) / 8, MSB_FIRST);
		 for (int i = 0; i < metadata_general->blocks; i++)
		 bitarray_clean_bit(bitmap, i);
		 actualizar_bit_map();*/
	}
	else {

		struct stat stats;
		fstat(fileno(bitmap_file), &stats);
		char* data = malloc(stats.st_size);
		fread(data, stats.st_size, 1, bitmap_file);
		fclose(bitmap_file);
		bitmap = bitarray_create_with_mode(data, stats.st_size, MSB_FIRST);

		/*u_int32_t size = metadata_general->blocks / 8;
		 char* data = malloc(size);
		 fread(data, size, 1, bitmap_file);
		 printf("SOY EL CHAR DATA: %s\n",data);
		 printf("ENTRE AL ELSE PORQUE ESTOY CREADO----------------\n");
		 fclose(bitmap_file);
		 bitmap = bitarray_create_with_mode(data, size, MSB_FIRST);*/
	}

	suscribirse_a_colas();

	pthread_create(&hilo_verificador_de_conexion, NULL, iniciar_hilo_verificador_de_conexion, NULL);

	pthread_create(&hilo_servidor, NULL, mantener_servidor, NULL);

	//fclose(archivo_metadata);
	//fclose(archivo_bitmap);
	pthread_join(hilo_verificador_de_conexion, NULL);
	pthread_join(hilo_new, NULL);
	pthread_join(hilo_get, NULL);
	pthread_join(hilo_catch, NULL);
	pthread_join(hilo_servidor, NULL);
	return 0;
}

