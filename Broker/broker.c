/*
 * broker.c
 *
 *  Created on: 17 abr. 2020
 *      Author: utnso
 */

#include<stdio.h>
#include<signal.h>
#include "broker.h"

void _dumpear_memoria(){
	generar_dump(memoria);
}

int main(void)
{
	signal(SIGINT, finalizar_servidor);
	signal(SIGUSR1, _dumpear_memoria);


	for(u_int32_t i = 0; i<6; i++){
			lista_suscriptores[i] = list_create();
			generador_id_suscriptor[i] = 1;
		}
	generador_id_mensaje = 1;

	config = config_create("./brokerFinal_PARTICIONES.config");

	uint32_t tamanio_memoria = config_get_int_value(config, "TAMANO_MEMORIA");
	tamano_minimo_particion = config_get_int_value(config, "TAMANO_MINIMO_PARTICION");
	algoritmo_memoria = config_get_string_value(config, "ALGORITMO_MEMORIA");
	algoritmo_particion_libre = config_get_string_value(config, "ALGORITMO_PARTICION_LIBRE");
	algoritmo_reemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
	frecuencia_compactacion = config_get_int_value(config, "FRECUENCIA_COMPACTACION");
	log_file = config_get_string_value(config, "LOG_FILE");
	timer_lru = 0;
	clock_compactacion = frecuencia_compactacion;
	ip = config_get_string_value(config, "IP_BROKER");
	puerto = config_get_string_value(config, "PUERTO_BROKER");


	memoria = crear_memoria(tamanio_memoria);
	cola_victimas = queue_create();



	logger = log_create("./broker.log", "Broker", 1, LOG_LEVEL_INFO);
	logger_auxiliar = log_create("./broker_auxiliar.log", "Broker Auxiliar", 1, LOG_LEVEL_INFO);
	log_warning(logger_auxiliar, "El ID del proceso es: %d\n",process_getpid());




	iniciar_servidor();

	return EXIT_SUCCESS;
}
