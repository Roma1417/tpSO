/*
 * broker.c
 *
 *  Created on: 17 abr. 2020
 *      Author: utnso
 */

#include<stdio.h>
#include "broker.h"



int main(void)
{
	printf("Empieza el Broker\n");
	for(u_int32_t i = 0; i<6; i++){
			colas_mensajes[i] = crear_cola_mensajes(i + 1);
			generador_id_suscriptor[i] = 1;
		}
	generador_id_mensaje = 1;

	t_config* config = config_create("broker.config");
	uint32_t tamanio_memoria = config_get_int_value(config, "TAMANO_MEMORIA");
	tamano_minimo_particion = config_get_int_value(config, "TAMANO_MINIMO_PARTICION");
	algoritmo_memoria = config_get_string_value(config, "ALGORITMO_MEMORIA");
	algoritmo_particion_libre = config_get_string_value(config, "ALGORITMO_PARTICION_LIBRE");
	algoritmo_reemplazo = config_get_string_value(config, "ALGORTIMO_REEMPLAZO");
	frecuencia_compactacion = config_get_int_value(config, "FRECUENCIA_COMPACTACION");
	log_file = config_get_string_value(config, "LOG_FILE");
	config_destroy(config);

	memoria = crear_memoria(tamanio_memoria);

	logger = log_create("./broker.log", "Broker", 0, LOG_LEVEL_INFO);




	//probando memoria



	/*agregar_stream(memoria, "asadsaf", strlen("asadsaf")+1);
	agregar_stream(memoria, "boca campeon", strlen("boca campeon")+1);
	agregar_stream(memoria, "chau", strlen("chau")+1);
	agregar_stream(memoria, "pepe", strlen("pepe")+1);
	agregar_stream(memoria, "qwert", strlen("qwert")+1);
	liberar_particion(memoria, 1);
	liberar_particion(memoria, 3);
	mostrar_memoria(memoria);
	printf("\n\nCompactada:");
	compactar_memoria(memoria);
	mostrar_memoria(memoria);*/





	iniciar_servidor();

	return EXIT_SUCCESS;
}
