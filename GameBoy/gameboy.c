/*
 * main.c
 *
 *  Created on: 28 feb. 2019
 *      Author: utnso
 */

#include "gameboy.h"

int main(void){

	int conexion;
	char* ip;
	char* puerto;

	t_log* logger;
	t_config* config;

	logger = iniciar_logger();

	config = leer_config();

	ip = config_get_string_value(config, "IP");
	puerto = config_get_string_value(config, "PUERTO");

	log_info(logger, "El IP es: %s", ip);
	log_info(logger, "El PUERTO es: %s", puerto);

	conexion = crear_conexion(ip,puerto);

	enviar_mensaje("Boca Campeon!", conexion);

	char* unMensaje = recibir_mensaje(conexion);

	log_info(logger, "El mensaje recibido es: %s", unMensaje);

	free(unMensaje);

	terminar_programa(conexion, logger, config);
}

t_log* iniciar_logger(void){
	t_log* logger = log_create("gameboy.log","gameboy", true, LOG_LEVEL_INFO);
	if(logger == NULL){
		 printf("No pude crear el logger\n");
		 exit(1);
	}
	return logger;
}

t_config* leer_config(void){
	t_config* config = config_create("./gameboy.config");
	if (config == NULL){
		printf("No pude leer la config\n");
		exit(1);
	}
	return config;
}

void terminar_programa(int conexion, t_log* logger, t_config* config){
	if (logger != NULL) log_destroy(logger);
	if (config != NULL) config_destroy(config);
	liberar_conexion(conexion);
}
