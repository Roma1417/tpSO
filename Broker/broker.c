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
	colas_mensajes[0] = crear_cola_mensajes(NEW_POKEMON);
	colas_mensajes[1] = crear_cola_mensajes(APPEARED_POKEMON);
	colas_mensajes[2] = crear_cola_mensajes(CATCH_POKEMON);
	colas_mensajes[3] = crear_cola_mensajes(CAUGHT_POKEMON);
	colas_mensajes[4] = crear_cola_mensajes(GET_POKEMON);
	colas_mensajes[5] = crear_cola_mensajes(LOCALIZED_POKEMON);

	generador_id_suscriptor[0] = 1;
	generador_id_suscriptor[1] = 1;
	generador_id_suscriptor[2] = 1;
	generador_id_suscriptor[3] = 1;
	generador_id_suscriptor[4] = 1;
	generador_id_suscriptor[5] = 1;

	generador_id_mensaje = 1;



	iniciar_servidor();

	return EXIT_SUCCESS;
}


