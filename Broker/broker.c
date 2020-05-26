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

	colas_mensajes[0] = cola_mensajes_create(NEW_POKEMON);
	colas_mensajes[1] = cola_mensajes_create(APPEARED_POKEMON);
	colas_mensajes[2] = cola_mensajes_create(CATCH_POKEMON);
	colas_mensajes[3] = cola_mensajes_create(CAUGHT_POKEMON);
	colas_mensajes[4] = cola_mensajes_create(GET_POKEMON);
	colas_mensajes[5] = cola_mensajes_create(LOCALIZED_POKEMON);


	iniciar_servidor();
	
	return EXIT_SUCCESS;
}


