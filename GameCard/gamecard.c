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

	t_log* logger = log_create("gamecard.log","gamecard", true, LOG_LEVEL_INFO);
	if(logger == NULL) {
		 printf("No pude crear el logger\n");
		 exit(1);
	}
	return logger;

}

void* mantener_servidor(){

	iniciar_servidor();

	return EXIT_SUCCESS;
}

char* id_cola_mensajes(char* msg){
	char* id_cola;
	if(string_equals_ignore_case(msg, "GET_POKEMON")) id_cola = string_itoa(id_cola_get);
	else if(string_equals_ignore_case(msg, "NEW_POKEMON")) id_cola = string_itoa(id_cola_new);
	else if(string_equals_ignore_case(msg, "CATCH_POKEMON")) id_cola = string_itoa(id_cola_catch);

	return id_cola;
}

void* suscribirse(void* cola){

	char* msg = (char *)cola;
	int conexion = crear_conexion(config_gamecard->ip_broker, config_gamecard->puerto_broker);
	char** mensaje = malloc(sizeof(char*)*4);
	printf("cola: %s\n", msg);
	mensaje[0] = string_new();
	string_append(&(mensaje[0]), "BROKER");

	mensaje[1] = string_new();
	string_append(&(mensaje[1]), "SUSCRIPTOR");

	mensaje[2] = string_new();
	string_append(&(mensaje[2]), msg);

	mensaje[3] = id_cola_mensajes(msg);

	// Y si establece conexion, no se envian mensajes bien

	enviar_mensaje(mensaje, conexion);

	//Falta esperar la respuesta

	//pthread_t thread_suscriptor;

	//serve_client(&conexion);


	printf("conexion: %d \n", conexion);

	/*while (1){
		pthread_create(&thread_suscriptor,NULL,(void*)serve_client,&conexion);
		pthread_join(thread_suscriptor, NULL);
	}*/
	while(1){
	pthread_create(&thread,NULL,(void*)serve_client,&conexion);
	pthread_join(thread, NULL);
	printf("id_new: %d\n", id_cola_new);
	printf("id_get: %d\n", id_cola_get);
	printf("id_catch: %d\n", id_cola_catch);
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
void suscribirse_a_colas(){

	// REVISAR
	char* mensaje = string_new();
	string_append(&mensaje, "NEW_POKEMON");
	pthread_create(&hilo_new, NULL, suscribirse,(void*) mensaje);

	char* mensaje2 = string_new();
	string_append(&mensaje2, "GET_POKEMON");
	pthread_create(&hilo_get, NULL, suscribirse,(void*) mensaje2);

	char* mensaje3 = string_new();
	string_append(&mensaje3, "CATCH_POKEMON");
	pthread_create(&hilo_catch, NULL, suscribirse,(void*) mensaje3);

}

/**
* @NAME: construir_config_gamecard
* @DESC: Crea y devuelve un puntero a una estructura t_config_gamecard,
* 		 donde sus valores internos corresponden a los guardados en
* 		 el archivo del gamecard de tipo config.
*/
t_config_gamecard* construir_config_gamecard(t_config* config){

	t_config_gamecard* config_gamecard_local = malloc(sizeof(t_config_gamecard));

	config_gamecard_local -> puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	config_gamecard_local -> ip_broker = config_get_string_value(config, "IP_BROKER");
	config_gamecard_local -> punto_montaje_tallgrass = config_get_string_value(config, "PUNTO_MONTAJE_TALLGRASS");

	config_gamecard_local -> tiempo_de_reintento_operacion = config_get_int_value(config, "TIEMPO_DE_REINTENTO_OPERACION");
	config_gamecard_local -> tiempo_de_reintento_conexion = config_get_int_value(config, "TIEMPO_DE_REINTENTO_CONEXION");


	return config_gamecard_local;
}

int main(){
	t_config* config = leer_config();
	//t_log* logger_team = iniciar_logger();
	id_cola_get = 0;
	id_cola_new = 0;
	id_cola_catch = 0;

	config_gamecard = construir_config_gamecard(config);

	suscribirse_a_colas();

	pthread_t hilo_servidor;
	pthread_create(&hilo_servidor, NULL, mantener_servidor, NULL);

	pthread_join(hilo_new, NULL);
	pthread_join(hilo_get, NULL);
	pthread_join(hilo_catch, NULL);
	pthread_join(hilo_servidor, NULL);
	return 0;
}

